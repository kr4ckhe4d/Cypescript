# EP05 — Control Flow: The Basic Block Puzzle

**Length target:** 24–28 min · **Repo anchor:** `CodeGen::visit(IfStatementNode*)`,
`WhileStatementNode`, `ForStatementNode`, `SwitchStatementNode`, `branchAndSealBlock`
**Goal:** if/else, while, for, break/continue, and switch — plus a real mental
model of basic blocks and terminators.

---

## [COLD OPEN] (0:00–0:35)

[SCREEN: IR of an if/else, with the blocks color-highlighted]
```llvm
  br i1 %cond, label %then, label %else
then:  ... br label %ifcont
else:  ... br label %ifcont
ifcont: ...
```

NARRATION:
> Machine code has no if statements. No while. No for. It has exactly one
> control-flow tool: *jump*. Today we compile all of structured programming
> down to labeled blocks and jumps — and I'll show you the one rule that,
> if you don't know it, will crash LLVM with the world's least helpful error.

---

## Segment 1 — Basic blocks and THE RULE (0:35–6:00)

[B-ROLL / DIAGRAM: a function as boxes (blocks) with arrows (branches)]

NARRATION:
> A **basic block** is a straight-line run of instructions: one way in at the
> top, one way out at the bottom. The instruction at the bottom is called the
> **terminator** — a branch, a conditional branch, or a return.
>
> Here's THE RULE. Write it on your wall:
>
> **Every basic block must end with exactly one terminator. Not zero. Not
> two.**
>
> Zero terminators: LLVM's verifier says "block does not have terminator" and
> refuses your module. Two: everything after the first is unreachable
> garbage and the verifier also screams. Half the bugs you'll write in this
> episode are violations of this rule. All of mine were.

---

## Segment 2 — if / else (6:00–12:00)

[TYPE: the pattern — this is the template for everything today]
```cpp
void CodeGen::visit(IfStatementNode* node) {
    llvm::Value* cond = ensureI1(visit(node->condition.get()));
    llvm::Function* fn = m_builder.GetInsertBlock()->getParent();

    auto* thenBB  = llvm::BasicBlock::Create(m_context, "then", fn);
    auto* elseBB  = llvm::BasicBlock::Create(m_context, "else", fn);
    auto* mergeBB = llvm::BasicBlock::Create(m_context, "ifcont", fn);

    m_builder.CreateCondBr(cond, thenBB, elseBB);

    m_builder.SetInsertPoint(thenBB);
    for (auto& s : node->thenStatements) visit(s.get());
    if (!m_builder.GetInsertBlock()->getTerminator())   // ← THE RULE
        m_builder.CreateBr(mergeBB);

    m_builder.SetInsertPoint(elseBB);
    for (auto& s : node->elseStatements) visit(s.get());
    if (!m_builder.GetInsertBlock()->getTerminator())
        m_builder.CreateBr(mergeBB);

    m_builder.SetInsertPoint(mergeBB);                  // life goes on here
}
```

NARRATION:
> The choreography: make three blocks — then, else, merge. Branch on the
> condition. Fill then, jump to merge. Fill else, jump to merge. Park the
> builder at merge so whatever comes next lands there.
>
> And see that guard — "if no terminator yet, branch to merge"? That's
> because the body might have ENDED with a return. If it did, adding our
> branch would be a second terminator. THE RULE, defended in code. You'll
> write this exact guard six more times today.

> One free gift: `else if` needs *zero* codegen work. The parser just puts a
> whole if statement inside the else branch. Structure recursion pays rent.

⚠ PITFALL (`ensureI1`):
> LLVM conditions must be `i1` — one bit. Your comparisons already produce
> i1, but truthiness of an integer needs `icmp ne x, 0`. Wrap it in a helper
> now (`ensureI1`) or you'll paste that compare in ten places.

---

## Segment 3 — while and for (12:00–17:00)

[B-ROLL / DIAGRAM: while = cond → body → back to cond → exit]

[SHOW: while — same template, blocks renamed]
```cpp
auto* condBB = ...("loopcond"); auto* bodyBB = ...("loopbody"); auto* exitBB = ...("loopexit");
m_builder.CreateBr(condBB);                    // fall INTO the condition
m_builder.SetInsertPoint(condBB);
m_builder.CreateCondBr(ensureI1(visit(cond)), bodyBB, exitBB);
m_builder.SetInsertPoint(bodyBB);
/* body */ 
m_builder.CreateBr(condBB);                    // loop!
m_builder.SetInsertPoint(exitBB);
```

NARRATION:
> A loop is just an if whose merge block is *behind* it. The backwards branch
> — body to condition — is the entire concept of iteration.
>
> For loops? A while wearing a hat: init block first, and a dedicated
> increment block between body and condition, so `continue` has somewhere to
> jump. [SHOW: repo's for-loop visitor briefly.] Do-while: enter at the body
> instead of the condition. Once you see the template, every loop is a
> rearrangement.

---

## Segment 4 — break and continue: the target stack (17:00–22:00)

NARRATION:
> Now the interesting one. `break` means "jump to the exit of the *nearest
> enclosing* loop." But when codegen meets a break, how does it know which
> loop it's inside? Answer: we tell it. With a stack.

[SHOW: from the repo]
```cpp
// {continueTarget, breakTarget} — pushed by every loop, popped after
std::vector<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>> loopTargets;
```
```cpp
// inside the while visitor, around the body:
loopTargets.push_back({condBB, exitBB});
for (auto& s : node->bodyStatements) visit(s.get());
loopTargets.pop_back();
```
```cpp
void CodeGen::visit(BreakStatementNode*) {
    if (loopTargets.empty()) throw std::runtime_error("'break' outside loop");
    branchAndSealBlock(loopTargets.back().second, "after_break");
}
```

[SHOW: `branchAndSealBlock` — and explain the dead block]
```cpp
void CodeGen::branchAndSealBlock(llvm::BasicBlock* target, const std::string& name) {
    m_builder.CreateBr(target);
    // park the builder in a fresh unreachable block, so code after `break`
    // has somewhere harmless to go
    auto* dead = llvm::BasicBlock::Create(m_context, name, currentFn);
    m_builder.SetInsertPoint(dead);
}
```

NARRATION:
> Two ideas here. The stack: loops push their targets entering, pop leaving —
> nesting handled automatically. And the dead block: after a break, the
> source might still say `println("unreachable")`. We can't emit that after
> a terminator — THE RULE — so we park the builder in a fresh block nothing
> jumps to. LLVM's optimizer deletes it later. Everybody's happy.

---

## Segment 5 — switch, with fallthrough (22:00–26:00)

[SHOW: the repo's switch codegen, explained at diagram level]

[B-ROLL / DIAGRAM: two parallel columns — "check chain" on the left
(case_check_0 → case_check_1 → default), "bodies" on the right
(case_body_0 → case_body_1 → ...), arrows from checks to bodies, and
*downward* arrows between bodies]

NARRATION:
> Switch looks scary, but it's two structures stapled together. A **chain of
> checks** — compare the value against each case, first match jumps to that
> case's body. And the **bodies in source order** — because if a body ends
> without a break, it falls *through* to the next body. That downward arrow
> IS fallthrough. Break? Already works — switch pushes onto the same target
> stack as loops, with the exit as its break target.
>
> Bonus: because our checks are just comparisons, `switch` on *strings* cost
> one extra branch calling strcmp. Try that in C.

---

## [OUTRO] (26:00–27:00)

> You now compile everything a structured language needs: decisions, loops,
> early exits. Next episode: functions — real ones with parameters and
> returns — plus the moment our language gets superpowers by linking against
> a C++ runtime library. Strings, arrays, and file I/O in one episode.

**Checkpoint tag:** `ep05-control-flow` · **Homework:** implement do-while.
It's a while with the entry branch moved. You have everything you need.
