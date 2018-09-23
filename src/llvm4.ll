; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %a = alloca [3 x i32]
  %0 = getelementptr [3 x i32], [3 x i32]* %a, i32 0, i32 0
  ret i32 0
}
