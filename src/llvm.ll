; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %a2 = alloca double
  %a1 = alloca double
  %a = alloca double
  store double 0x3FF19999A0000000, double* %a
  %0 = load double, double* %a
  %eqtmp = fcmp ueq double %0, 1.000000e+00
  %1 = sitofp i1 %eqtmp to double
  %ifcond = fcmp one double %1, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  store double 2.000000e+00, double* %a1
  br label %ifcont

else:                                             ; preds = %entry
  store double 0x4008CCCCC0000000, double* %a2
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi float [ 2.000000e+00, %then ], [ 0x4008CCCCC0000000, %else ]
  %2 = load double, double* %a2
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @1, i32 0, i32 0), double %2)
  ret i32 0
}
