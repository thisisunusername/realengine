#tips SHELL_MAIN //表明这是一个SHELL,跟c的#define _GNU_SOURCE差不多
#import <linux.reh>
#import <pwd.reh>
#import <base.reh>
#import <gnu.reh>
#import <fs.reh>
#import <text.reh>
#import <proc.reh>
#import <net.reh>
function_add "main1"(type=main_){
get_keyboard ABCDEFGHIJKLMNOPQRSTUVWXYZ,abcdefghijklmnopqrstuvwxyz,1234567890,$";:!?#[].'"$; //这里的$""$代表只有里面的东西，$"""$只算"
}
function_add "help"(type=normal){
printf("RealEngine shell (ashell.re) v0.2.0\n");
printf("-------------------- builtin --------------------\n");
printf("  help            show this message\n");
printf("  exit            quit the shell\n");
printf("  cd      <dir>   change directory\n");
printf("  pwd             print working directory\n");
printf("  echo    <text>  print text\n");
printf("  exec    <cmd>   run a command via /bin/sh -c\n");
printf("-------------------- coreutils ------------------\n");
printf("  ls cp mv rm mkdir touch stat du\n");
printf("  cat head grep wc sort uniq cut tr sed awk diff tee seq\n");
printf("  ps kill env date uptime df free hostname\n");
printf("  ping curl wget ip which sleep true false\n");
}
function_add "builtin"(type=normal){
if $INPUT is "pwd";then;do="printf("$execute_command pwd\n");";end_if;
if $INPUT is "echo";then;do="printf("$INPUT_OPT\n");";end_if;
if $INPUT is "exec";then;do="execve('/bin/sh', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "cd";then;do="execve('/bin/sh', '-c', 'cd $INPUT_OPT && pwd')";end_if;
if $INPUT is "ls";then;do="execve('/bin/ls', '--color=never', '$INPUT_OPT')";end_if;
if $INPUT is "cp";then;do="execve('/bin/cp', '-rv', '$INPUT_OPT')";end_if;
if $INPUT is "mv";then;do="execve('/bin/mv', '-v', '$INPUT_OPT')";end_if;
if $INPUT is "rm";then;do="execve('/bin/rm', '-rf', '$INPUT_OPT')";end_if;
if $INPUT is "mkdir";then;do="execve('/bin/mkdir', '-p', '$INPUT_OPT')";end_if;
if $INPUT is "touch";then;do="execve('/usr/bin/touch', '$INPUT_OPT')";end_if;
if $INPUT is "stat";then;do="execve('/usr/bin/stat', '$INPUT_OPT')";end_if;
if $INPUT is "du";then;do="execve('/usr/bin/du', '-sh', '$INPUT_OPT')";end_if;
if $INPUT is "cat";then;do="execve('/bin/cat', '$INPUT_OPT')";end_if;
if $INPUT is "head";then;do="execve('/usr/bin/head', '-n', '20', '$INPUT_OPT')";end_if;
if $INPUT is "grep";then;do="execve('/bin/grep', '--color=never', '$INPUT_OPT')";end_if;
if $INPUT is "wc";then;do="execve('/usr/bin/wc', '-l', '-w', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "sort";then;do="execve('/usr/bin/sort', '$INPUT_OPT')";end_if;
if $INPUT is "uniq";then;do="execve('/usr/bin/uniq', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "cut";then;do="execve('/usr/bin/cut', '$INPUT_OPT')";end_if;
if $INPUT is "tr";then;do="execve('/usr/bin/tr', '$INPUT_OPT')";end_if;
if $INPUT is "sed";then;do="execve('/bin/sed', '$INPUT_OPT')";end_if;
if $INPUT is "awk";then;do="execve('/usr/bin/awk', '$INPUT_OPT')";end_if;
if $INPUT is "diff";then;do="execve('/usr/bin/diff', '-u', '$INPUT_OPT')";end_if;
if $INPUT is "tee";then;do="execve('/usr/bin/tee', '$INPUT_OPT')";end_if;
if $INPUT is "seq";then;do="execve('/usr/bin/seq', '$INPUT_OPT')";end_if;
if $INPUT is "ps";then;do="execve('/bin/ps', 'aux')";end_if;
if $INPUT is "kill";then;do="execve('/bin/kill', '$INPUT_OPT')";end_if;
if $INPUT is "env";then;do="execve('/usr/bin/env')";end_if;
if $INPUT is "date";then;do="execve('/bin/date', '$INPUT_OPT')";end_if;
if $INPUT is "uptime";then;do="execve('/usr/bin/uptime')";end_if;
if $INPUT is "df";then;do="execve('/bin/df', '-h')";end_if;
if $INPUT is "free";then;do="execve('/usr/bin/free', '-h')";end_if;
if $INPUT is "hostname";then;do="execve('/bin/hostname')";end_if;
if $INPUT is "ping";then;do="execve('/bin/ping', '-c', '4', '$INPUT_OPT')";end_if;
if $INPUT is "curl";then;do="execve('/usr/bin/curl', '-sSL', '$INPUT_OPT')";end_if;
if $INPUT is "wget";then;do="execve('/usr/bin/wget', '-qO-', '$INPUT_OPT')";end_if;
if $INPUT is "ip";then;do="execve('/sbin/ip', 'addr')";end_if;
if $INPUT is "which";then;do="execve('/usr/bin/which', '$INPUT_OPT')";end_if;
if $INPUT is "sleep";then;do="execve('/bin/sleep', '$INPUT_OPT')";end_if;
if $INPUT is "true";then;do="execve('/bin/true')";end_if;
if $INPUT is "false";then;do="execve('/bin/false')";end_if;
if $INPUT is "whoami";then;do="execve('/usr/bin/whoami')";end_if;
if $INPUT is "uname";then;do="execve('/usr/bin/uname', '-a')";end_if;
if $INPUT is "id";then;do="execve('/usr/bin/id')";end_if;
}
function_add "main"(type=main){
printf("$execute_command whoami");
printf("@localhost ");
printf("$execute_command pwd");
printf(" # ");
wait_input;
if $INPUT is "help";then;do="start_function "help";";end_if;
if $INPUT is "exit";then;do="kill";end_if;
start_function "builtin";
start_function "main";
}
start_function "main";add_other_function "help,builtin"
//End Of Program //注意每个程序结尾都有End Of Program，不然认不出这是RealEngine