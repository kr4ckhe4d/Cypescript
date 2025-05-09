; ModuleID = 'MyLangModule'
source_filename = "MyLangModule"

@.str_literal = private unnamed_addr constant [23 x i8] c"Hello from a variable!\00", align 1
@.str_literal.1 = private unnamed_addr constant [16 x i8] c"Second message.\00", align 1

define i32 @main() {
entry:
  %x = alloca ptr, align 8
  %greeting = alloca ptr, align 8
  store ptr @.str_literal, ptr %greeting, align 8
  %greeting_val = load ptr, ptr %greeting, align 8
  %putsCall = call i32 @puts(ptr %greeting_val)
  store ptr @.str_literal.1, ptr %x, align 8
  %x_val = load ptr, ptr %x, align 8
  %putsCall1 = call i32 @puts(ptr %x_val)
  ret i32 0
}

declare i32 @puts(ptr)
