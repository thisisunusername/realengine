#tips SHELL_MAIN //表明这是一个SHELL,跟c的#define _GNU_SOURCE差不多
#import <linux.reh>
#import <pwd.reh>
#import <base.reh>
#import <gnu.reh> //这些都是useless的import,如果你想，你可以加入头文件
function_add "main1"(type=main_){
get_keyboard ABCDEFGHIJKLMNOPQRSTUVWXYZ,abcdefghijklmnopqrstuvwxyz,1234567890,$";:!?#[]."'"$; //这里的$""$代表只有里面的东西，$"""$只算"
}
function_add "help"(type=normal){
if $INPUT is "help";then;do="printf("A shell by RealEngine \n Built-in command:help,exec,pwd,echo");";
}
function_add "builtin"(type=normal){
if $INPUT is "exec";then do="execve "$INPUT_OPT"";else;do="printf("No command input.Try exec /bin/busybox");";end_if;if $INPUT is "pwd";then;do="printf("$execute_command pwd_cat");";end_if;if $INPUT is echo;then;do="printf("$INPUT_OPT");";else;do="printf("\n\n");";end_if; //INPUTOPT指的是选项,比如echo xxx,xxx是echo的选项，$execute_command和bash的$()一样
}
function_add "main"(type=main){
if $INPUT is "$execute_command search $PATH"*filename;then;do="execute $INPUT;";end_if;form_function="main1";wait_input;set PS1=""$execute_command system("whoami")"  +  @  +  "localhost"  +  "$execute_command pwd_cat"  +  "$execute_command if geteuid() == "0";then;do"printf("#")";else;do="printf("$")";";";
}
join_function "main";add_other_function "help,builtin"
//End Of Program //注意每个程序结尾都有End Of Program，不然别人认不出这是RealEngine