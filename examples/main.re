// ====== 用户原稿（保持原样，只补了 do= 的语法修正）======
function_add "main"(type=main){
  create_loop "main"; in loop "main" do="
    execve("./libs/Main")
    wait 3 second
    execve("./libs/Main", "add_lib", "./libmain.so", "./libv4.so", "./libc.so", "./libcore.so")
    if $up_cmd_exitcode is 0; then do="kill 'main'"; else do="printf('Launch failed,Please check your libs directory.')"
  "
  start_loop "main"
}
start_function "main"
exit //End Of Program
