; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %i = alloca i32
  %b = alloca double
  %a = alloca [6 x double]
  %0 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 0
  store double 1.000000e+00, double* %0
  %1 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 1
  store double 2.000000e+00, double* %1
  %2 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 2
  store double 3.000000e+00, double* %2
  %3 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 3
  store double 3.000000e+00, double* %3
  %4 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 4
  store double 0x4000CCCCC0000000, double* %4
  %5 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 5
  store double 1.000000e+00, double* %5
  store double 0.000000e+00, double* %b
  store i32 0, i32* %i
  br label %loop

loop:                                             ; preds = %loop, %entry
  %6 = load double, double* %b
  %7 = load i32, i32* %i
  %8 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 %7
  %9 = load double, double* %8
  %addtmp = fadd double %6, %9
  store double %addtmp, double* %b
  %10 = load i32, i32* %i
  %nextvar = add i32 %10, 1
  store i32 %nextvar, i32* %i
  %leqtmp = icmp slt i32 %10, 5
  %11 = sitofp i1 %leqtmp to double
  %loopcond = fcmp one double %11, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  %12 = load double, double* %b
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @1, i32 0, i32 0), double %12)
  ret i32 0
}
