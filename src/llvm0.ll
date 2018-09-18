; ModuleID = 'Module'
source_filename = "Module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
}

define i32 @abc(i32 %a, i32 %b) {
entry:
  %b2 = alloca i32
  %a1 = alloca i32
  store i32 %a, i32* %a1
  store i32 %b, i32* %b2
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  ret i32 1
}

define i32 @cda(i32 %d, i32 %e) {
entry:
  %e2 = alloca i32
  %d1 = alloca i32
  store i32 %d, i32* %d1
  store i32 %e, i32* %e2
  %printfCall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  ret i32 1
  %printfCall3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 312)
  ret i32 0
}
