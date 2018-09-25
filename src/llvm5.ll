; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@3 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define double @abc(i32 %n) {
entry:
  %n1 = alloca i32
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %addtmp = add i32 %0, 10
  %1 = sitofp i32 %addtmp to double
  ret double %1
}

define i32 @main() {
entry:
  %i = alloca i32
  %a = alloca [6 x double]
  %calltmp = call double @abc(i32 10)
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), double %calltmp)
  %0 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 0
  store double 1.000000e+00, double* %0
  %1 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 1
  store double 2.000000e+00, double* %1
  %2 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 2
  store double 0x4008CCCCC0000000, double* %2
  %3 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 3
  store double 0x40091EB860000000, double* %3
  %4 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 4
  store double 0x400920C4A0000000, double* %4
  %5 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 5
  store double 0x400921CAC0000000, double* %5
  %6 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 1
  %7 = load double, double* %6
  store double 3.000000e+00, double* %6
  store i32 0, i32* %i
  br label %loop

loop:                                             ; preds = %loop, %entry
  %8 = load i32, i32* %i
  %9 = getelementptr [6 x double], [6 x double]* %a, i32 0, i32 %8
  %10 = load double, double* %9
  %printfCall1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), double %10)
  %11 = load i32, i32* %i
  %nextvar = add i32 %11, 1
  store i32 %nextvar, i32* %i
  %leqtmp = icmp slt i32 %11, 5
  %12 = sitofp i1 %leqtmp to double
  %loopcond = fcmp one double %12, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  ret i32 0
}
