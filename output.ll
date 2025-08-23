; ModuleID = 'CypescriptModule'
source_filename = "CypescriptModule"

@.str_literal = private unnamed_addr constant [12 x i8] c"a equals 10\00", align 1
@.str_literal.1 = private unnamed_addr constant [20 x i8] c"b does not equal 10\00", align 1
@.str_literal.2 = private unnamed_addr constant [17 x i8] c"b is less than a\00", align 1
@.str_literal.3 = private unnamed_addr constant [20 x i8] c"a is greater than b\00", align 1
@.str_literal.4 = private unnamed_addr constant [21 x i8] c"a is greater than 20\00", align 1
@.str_literal.5 = private unnamed_addr constant [25 x i8] c"a is not greater than 20\00", align 1
@.str_literal.6 = private unnamed_addr constant [10 x i8] c"Sum is 15\00", align 1
@.str_literal.7 = private unnamed_addr constant [14 x i8] c"Sum is not 15\00", align 1
@.str_literal.8 = private unnamed_addr constant [17 x i8] c"a > 5 and b < 10\00", align 1
@.str_literal.9 = private unnamed_addr constant [11 x i8] c"x equals y\00", align 1
@.str_literal.10 = private unnamed_addr constant [26 x i8] c"x less than or equal to y\00", align 1
@.str_literal.11 = private unnamed_addr constant [29 x i8] c"x greater than or equal to y\00", align 1
@.str_literal.12 = private unnamed_addr constant [12 x i8] c"Math works!\00", align 1
@.str_literal.13 = private unnamed_addr constant [30 x i8] c"Division and comparison work!\00", align 1

define i32 @main() {
entry:
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  %result = alloca i32, align 4
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4
  store i32 5, ptr %b, align 4
  %a_val = load i32, ptr %a, align 4
  %eqtmp = icmp eq i32 %a_val, 10
  br i1 %eqtmp, label %then, label %ifcont

then:                                             ; preds = %entry
  %putsCall = call i32 @puts(ptr @.str_literal)
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  %b_val = load i32, ptr %b, align 4
  %netmp = icmp ne i32 %b_val, 10
  br i1 %netmp, label %then1, label %ifcont2

then1:                                            ; preds = %ifcont
  %putsCall3 = call i32 @puts(ptr @.str_literal.1)
  br label %ifcont2

ifcont2:                                          ; preds = %then1, %ifcont
  %b_val4 = load i32, ptr %b, align 4
  %a_val5 = load i32, ptr %a, align 4
  %lttmp = icmp slt i32 %b_val4, %a_val5
  br i1 %lttmp, label %then6, label %ifcont7

then6:                                            ; preds = %ifcont2
  %putsCall8 = call i32 @puts(ptr @.str_literal.2)
  br label %ifcont7

ifcont7:                                          ; preds = %then6, %ifcont2
  %a_val9 = load i32, ptr %a, align 4
  %b_val10 = load i32, ptr %b, align 4
  %gttmp = icmp sgt i32 %a_val9, %b_val10
  br i1 %gttmp, label %then11, label %ifcont12

then11:                                           ; preds = %ifcont7
  %putsCall13 = call i32 @puts(ptr @.str_literal.3)
  br label %ifcont12

ifcont12:                                         ; preds = %then11, %ifcont7
  %a_val14 = load i32, ptr %a, align 4
  %gttmp15 = icmp sgt i32 %a_val14, 20
  br i1 %gttmp15, label %then16, label %else

then16:                                           ; preds = %ifcont12
  %putsCall18 = call i32 @puts(ptr @.str_literal.4)
  br label %ifcont17

ifcont17:                                         ; preds = %else, %then16
  %a_val20 = load i32, ptr %a, align 4
  %b_val21 = load i32, ptr %b, align 4
  %addtmp = add i32 %a_val20, %b_val21
  store i32 %addtmp, ptr %result, align 4
  %result_val = load i32, ptr %result, align 4
  %eqtmp22 = icmp eq i32 %result_val, 15
  br i1 %eqtmp22, label %then23, label %else25

else:                                             ; preds = %ifcont12
  %putsCall19 = call i32 @puts(ptr @.str_literal.5)
  br label %ifcont17

then23:                                           ; preds = %ifcont17
  %putsCall26 = call i32 @puts(ptr @.str_literal.6)
  br label %ifcont24

ifcont24:                                         ; preds = %else25, %then23
  %a_val28 = load i32, ptr %a, align 4
  %gttmp29 = icmp sgt i32 %a_val28, 5
  br i1 %gttmp29, label %then30, label %ifcont31

else25:                                           ; preds = %ifcont17
  %putsCall27 = call i32 @puts(ptr @.str_literal.7)
  br label %ifcont24

then30:                                           ; preds = %ifcont24
  %b_val32 = load i32, ptr %b, align 4
  %lttmp33 = icmp slt i32 %b_val32, 10
  br i1 %lttmp33, label %then34, label %ifcont35

ifcont31:                                         ; preds = %ifcont35, %ifcont24
  store i32 7, ptr %x, align 4
  store i32 7, ptr %y, align 4
  %x_val = load i32, ptr %x, align 4
  %y_val = load i32, ptr %y, align 4
  %eqtmp37 = icmp eq i32 %x_val, %y_val
  br i1 %eqtmp37, label %then38, label %ifcont39

then34:                                           ; preds = %then30
  %putsCall36 = call i32 @puts(ptr @.str_literal.8)
  br label %ifcont35

ifcont35:                                         ; preds = %then34, %then30
  br label %ifcont31

then38:                                           ; preds = %ifcont31
  %putsCall40 = call i32 @puts(ptr @.str_literal.9)
  br label %ifcont39

ifcont39:                                         ; preds = %then38, %ifcont31
  %x_val41 = load i32, ptr %x, align 4
  %y_val42 = load i32, ptr %y, align 4
  %letmp = icmp sle i32 %x_val41, %y_val42
  br i1 %letmp, label %then43, label %ifcont44

then43:                                           ; preds = %ifcont39
  %putsCall45 = call i32 @puts(ptr @.str_literal.10)
  br label %ifcont44

ifcont44:                                         ; preds = %then43, %ifcont39
  %x_val46 = load i32, ptr %x, align 4
  %y_val47 = load i32, ptr %y, align 4
  %getmp = icmp sge i32 %x_val46, %y_val47
  br i1 %getmp, label %then48, label %ifcont49

then48:                                           ; preds = %ifcont44
  %putsCall50 = call i32 @puts(ptr @.str_literal.11)
  br label %ifcont49

ifcont49:                                         ; preds = %then48, %ifcont44
  br i1 true, label %then51, label %ifcont52

then51:                                           ; preds = %ifcont49
  %putsCall53 = call i32 @puts(ptr @.str_literal.12)
  br label %ifcont52

ifcont52:                                         ; preds = %then51, %ifcont49
  br i1 true, label %then54, label %ifcont55

then54:                                           ; preds = %ifcont52
  %putsCall56 = call i32 @puts(ptr @.str_literal.13)
  br label %ifcont55

ifcont55:                                         ; preds = %then54, %ifcont52
  ret i32 0
}

declare i32 @puts(ptr)
