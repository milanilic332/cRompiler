; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [7 x i8] c"%.2lf\0A\00"
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@3 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @fac(i32 %n) {
entry:
  %n1 = alloca i32
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %eqtmp = icmp eq i32 %0, 1
  %1 = load i32, i32* %n1
  %eqtmp2 = icmp eq i32 %1, 0
  %ortmp = or i1 %eqtmp, %eqtmp2
  %2 = sitofp i1 %ortmp to double
  %ifcond = fcmp one double %2, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  %3 = load i32, i32* %n1
  %4 = load i32, i32* %n1
  %subtmp = sub i32 %4, 1
  %calltmp = call i32 @fac(i32 %subtmp)
  %multmp = mul i32 %3, %calltmp
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 1, %then ], [ %multmp, %else ]
  ret i32 %iftmp
}

define i32 @main() {
entry:
  %calltmp = call i32 @fac(i32 5)
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), i32 %calltmp)
  ret i32 0
}
