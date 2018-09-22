; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@3 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @fac(i32 %n) {
entry:
  %n1 = alloca i32
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %eqtmp = icmp eq i32 %0, 0
  %1 = sitofp i1 %eqtmp to double
  %ifcond = fcmp one double %1, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  %2 = load i32, i32* %n1
  %subtmp = sub i32 %2, 1
  %calltmp = call i32 @fac(i32 %subtmp)
  %addtmp = add i32 1, %calltmp
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 1, %then ], [ %addtmp, %else ]
  ret i32 %iftmp
}

define i32 @main() {
entry:
  %calltmp = call i32 @fac(i32 3)
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), i32 %calltmp)
  ret i32 0
}
