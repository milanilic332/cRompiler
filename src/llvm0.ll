; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [7 x i8] c"%.2lf\0A\00"
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@3 = private unnamed_addr constant [7 x i8] c"%.2lf\0A\00"
@4 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@5 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @fibRek(i32 %n) {
entry:
  %n1 = alloca i32
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %eqtmp = icmp eq i32 %0, 1
  %1 = load i32, i32* %n1
  %eqtmp2 = icmp eq i32 %1, 2
  %ortmp = or i1 %eqtmp, %eqtmp2
  %2 = sitofp i1 %ortmp to double
  %ifcond = fcmp one double %2, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  %3 = load i32, i32* %n1
  %subtmp = sub i32 %3, 1
  %calltmp = call i32 @fibRek(i32 %subtmp)
  %4 = load i32, i32* %n1
  %subtmp3 = sub i32 %4, 2
  %calltmp4 = call i32 @fibRek(i32 %subtmp3)
  %addtmp = add i32 %calltmp, %calltmp4
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 1, %then ], [ %addtmp, %else ]
  ret i32 %iftmp
}

define i32 @fibIt(i32 %n) {
entry:
  %tmp = alloca i32
  %i = alloca i32
  %b = alloca i32
  %a = alloca i32
  %n1 = alloca i32
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %eqtmp = icmp eq i32 %0, 1
  %1 = load i32, i32* %n1
  %eqtmp2 = icmp eq i32 %1, 2
  %ortmp = or i1 %eqtmp, %eqtmp2
  %2 = sitofp i1 %ortmp to double
  %ifcond = fcmp one double %2, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 1, %then ], [ 0, %else ]
  store i32 1, i32* %a
  store i32 1, i32* %b
  store i32 3, i32* %i
  br label %loop

loop:                                             ; preds = %loop, %ifcont
  %3 = load i32, i32* %b
  store i32 %3, i32* %tmp
  %4 = load i32, i32* %a
  %5 = load i32, i32* %b
  %addtmp = add i32 %4, %5
  store i32 %addtmp, i32* %b
  %6 = load i32, i32* %tmp
  store i32 %6, i32* %a
  %7 = load i32, i32* %i
  %nextvar = add i32 %7, 1
  store i32 %nextvar, i32* %i
  %8 = load i32, i32* %n1
  %leqtmp = icmp slt i32 %7, %8
  %9 = sitofp i1 %leqtmp to double
  %loopcond = fcmp one double %9, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  %10 = load i32, i32* %b
  ret i32 %10
}

define i32 @main() {
entry:
  %calltmp = call i32 @fibRek(i32 10)
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), i32 %calltmp)
  %calltmp1 = call i32 @fibIt(i32 10)
  %printfCall2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), i32 %calltmp1)
  ret i32 0
}
