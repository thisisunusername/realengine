// 验证所有语法特性
printf("=== 测试开始 ===")

// (1) 定义函数，用花括号块
function_add "greet"(type=main){
  printf("我是 greet 函数")
}

// (2) 定义函数，用 do= 字符串块（原稿风格）
function_add "work"(type=main) do="
  printf('work 开始干活')
  execve("/bin/echo", "RealEngine", "跑起来了")
  wait 1 second
  printf('work 干完了')
"

// (3) 创建循环（原稿同款写法）
create_loop "main"; in loop "main" do="
  printf('-- 循环体第 1 步 --')
  printf('-- 循环体第 2 步 --')
"

// (4) 赋值
let $up_cmd_exitcode = 0

printf("=== 开始启动函数 ===")
start_function "greet"
start_function "work"

// (5) 判断，带 else
if $up_cmd_exitcode is 0; then do="printf('退出码是 0，成功')"; else do="printf('退出码不是 0，失败')"

// (6) 启动循环
start_loop "main"

printf("=== 全部结束 ===")
exit
