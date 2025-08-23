; ModuleID = 'CypescriptModule'
source_filename = "CypescriptModule"

@.str_literal = private unnamed_addr constant [25 x i8] c"Hello, Cypescript World!\00", align 1
@.format_int = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str_literal.1 = private unnamed_addr constant [11 x i8] c"Cypescript\00", align 1
@.str_literal.2 = private unnamed_addr constant [8 x i8] c"3.14159\00", align 1
@.format_int.3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str_literal.4 = private unnamed_addr constant [22 x i8] c"Direct string literal\00", align 1
@.format_int.5 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str_literal.6 = private unnamed_addr constant [16 x i8] c"Another message\00", align 1
@.str_literal.7 = private unnamed_addr constant [10 x i8] c"Developer\00", align 1
@.format_int.8 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.format_int.9 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.format_int.10 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %large_number = alloca i32, align 4
  %decimal = alloca i32, align 4
  %user_name = alloca ptr, align 8
  %magic_number = alloca i32, align 4
  %pi = alloca ptr, align 8
  %version = alloca i32, align 4
  %name = alloca ptr, align 8
  %answer = alloca i32, align 4
  %greeting = alloca ptr, align 8
  store ptr @.str_literal, ptr %greeting, align 8
  %greeting_val = load ptr, ptr %greeting, align 8
  %putsCall = call i32 @puts(ptr %greeting_val)
  store i32 42, ptr %answer, align 4
  %answer_val = load i32, ptr %answer, align 4
  %printfCall = call i32 (ptr, ...) @printf(ptr @.format_int, i32 %answer_val)
  store ptr @.str_literal.1, ptr %name, align 8
  store i32 1, ptr %version, align 4
  store ptr @.str_literal.2, ptr %pi, align 8
  %name_val = load ptr, ptr %name, align 8
  %putsCall1 = call i32 @puts(ptr %name_val)
  %version_val = load i32, ptr %version, align 4
  %printfCall2 = call i32 (ptr, ...) @printf(ptr @.format_int.3, i32 %version_val)
  %pi_val = load ptr, ptr %pi, align 8
  %putsCall3 = call i32 @puts(ptr %pi_val)
  %putsCall4 = call i32 @puts(ptr @.str_literal.4)
  %printfCall5 = call i32 (ptr, ...) @printf(ptr @.format_int.5, i32 999)
  %putsCall6 = call i32 @puts(ptr @.str_literal.6)
  store i32 123, ptr %magic_number, align 4
  store ptr @.str_literal.7, ptr %user_name, align 8
  %magic_number_val = load i32, ptr %magic_number, align 4
  %printfCall7 = call i32 (ptr, ...) @printf(ptr @.format_int.8, i32 %magic_number_val)
  %user_name_val = load ptr, ptr %user_name, align 8
  %putsCall8 = call i32 @puts(ptr %user_name_val)
  store i32 100, ptr %decimal, align 4
  store i32 987654321, ptr %large_number, align 4
  %decimal_val = load i32, ptr %decimal, align 4
  %printfCall9 = call i32 (ptr, ...) @printf(ptr @.format_int.9, i32 %decimal_val)
  %large_number_val = load i32, ptr %large_number, align 4
  %printfCall10 = call i32 (ptr, ...) @printf(ptr @.format_int.10, i32 %large_number_val)
  ret i32 0
}

declare i32 @puts(ptr)

declare i32 @printf(ptr, ...)
