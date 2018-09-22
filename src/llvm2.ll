; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@3 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @abc(double %a, i32 %b) {
entry:
  %b2 = alloca i32
  %a1 = alloca double
  store double %a, double* %a1
  store i32 %b, i32* %b2
  %0 = load double, double* %a1
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @1, i32 0, i32 0), double %0)
  %1 = load i32, i32* %b2
  %printfCall3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %1)
  ret i32 %1
}

define i32 @main() {
entry:
  %a = alloca i32
  %calltmp = call i32 @abc(double 0x3FF19999A0000000, i32 2)
  store i32 %calltmp, i32* %a
  ret i32 0
}
