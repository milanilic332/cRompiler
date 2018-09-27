; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [5 x i8] c"%lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %b = alloca i32
  %a = alloca i32
  store i32 3, i32* %a
  store i32 3, i32* %b
  %0 = load i32, i32* %a
  %eqtmp = icmp eq i32 %0, 3
  %1 = sitofp i1 %eqtmp to double
  %ifcond = fcmp one double %1, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  store i32 2, i32* %b
  br label %ifcont

else:                                             ; preds = %entry
  store i32 5, i32* %b
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 2, %then ], [ 5, %else ]
  %2 = load i32, i32* %a
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %2)
  %3 = load i32, i32* %b
  %printfCall1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %3)
  br label %loop

loop:                                             ; preds = %loop, %ifcont
  %4 = load i32, i32* %b
  %printfCall2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %4)
  %5 = load i32, i32* %b
  %addtmp = add i32 %5, 1
  store i32 %addtmp, i32* %b
  %6 = load i32, i32* %b
  %7 = sitofp i32 %6 to double
  %lttmp = fcmp ult double %7, 0x4016147AE0000000
  %8 = sitofp i1 %lttmp to double
  %loopcond = fcmp one double %8, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  ret i32 0
}
