; ModuleID = 'MyLangModule'
source_filename = "MyLangModule"

@.str_literal = private unnamed_addr constant [22 x i8] c"The magic number is: \00", align 1
@.format_int = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.format_int.1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.format_int.2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %another_num = alloca i32, align 4
  %magic_number = alloca i32, align 4
  %message = alloca ptr, align 8
  store ptr @.str_literal, ptr %message, align 8
  %message_val = load ptr, ptr %message, align 8
  %putsCall = call i32 @puts(ptr %message_val)
  store i32 42, ptr %magic_number, align 4
  %magic_number_val = load i32, ptr %magic_number, align 4
  %printfCall = call i32 (ptr, ...) @printf(ptr @.format_int, i32 %magic_number_val)
  store i32 123, ptr %another_num, align 4
  %another_num_val = load i32, ptr %another_num, align 4
  %printfCall1 = call i32 (ptr, ...) @printf(ptr @.format_int.1, i32 %another_num_val)
  %printfCall2 = call i32 (ptr, ...) @printf(ptr @.format_int.2, i32 789)
  ret i32 0
}

declare i32 @puts(ptr)

declare i32 @printf(ptr, ...)
