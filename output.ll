; ModuleID = 'MyLangModule'
source_filename = "MyLangModule"

@.str = private unnamed_addr constant [14 x i8] c"Hello, World!\00", align 1

define i32 @main() {
entry:
  %putsCall = call i32 @puts(ptr @.str)
  ret i32 0
}

declare i32 @puts(ptr)
