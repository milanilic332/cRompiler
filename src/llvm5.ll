; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %i = alloca i32
  %b = alloca double
  %step = alloca double
  %end = alloca double
  %start = alloca double
  %idx = alloca i32
  %a = alloca <128 x double>
  store i32 0, i32* %idx
  store double 1.000000e+00, double* %start
  store double 1.000000e+01, double* %end
  store double 0x3FF4CCCCC0000000, double* %step
  br label %loop

loop:                                             ; preds = %loop, %entry
  %0 = load double, double* %start
  %1 = load double, double* %end
  %2 = load double, double* %step
  %3 = load i32, i32* %idx
  %4 = getelementptr <128 x double>, <128 x double>* %a, i32 0, i32 %3
  store double %0, double* %4
  %addtmp = fadd double %0, %2
  %addtmp1 = add i32 %3, 1
  store i32 %addtmp1, i32* %idx
  store double %addtmp, double* %start
  %leqtmp = fcmp ule double %addtmp, %1
  %5 = sitofp i1 %leqtmp to double
  %loopcond = fcmp one double %5, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  store double 0x401BB13B24BBD5A0, double* %b
  store i32 0, i32* %i
  br label %loop2

loop2:                                            ; preds = %loop2, %afterloop
  %6 = load i32, i32* %i
  %7 = getelementptr <128 x double>, <128 x double>* %a, i32 0, i32 %6
  %8 = load double, double* %7
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @1, i32 0, i32 0), double %8)
  %9 = load i32, i32* %i
  %nextvar = add i32 %9, 1
  store i32 %nextvar, i32* %i
  %10 = load double, double* %b
  %11 = fptosi double %10 to i32
  %leqtmp3 = icmp slt i32 %9, %11
  %12 = sitofp i1 %leqtmp3 to double
  %loopcond4 = fcmp one double %12, 0.000000e+00
  br i1 %loopcond4, label %loop2, label %afterloop5

afterloop5:                                       ; preds = %loop2
  ret i32 0
}
