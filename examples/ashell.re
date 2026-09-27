#tips SHELL_MAIN //表明这是一个SHELL,跟c的#define _GNU_SOURCE差不多
#import <linux.reh>
#import <pwd.reh>
#import <base.reh>
#import <gnu.reh>
#import <fs.reh>
#import <text.reh>
#import <proc.reh>
#import <net.reh>
// 说明：所有外部命令都用 $BIN_xxx 动态路径，由解释器在启动时按
//       $RE_BIN_DIR > $PREFIX/bin > $PATH > /usr/bin 探测得到。
//       这样同一份脚本在 Termux($PREFIX/bin) 和 glibc 系统(/usr/bin) 都能跑。
function_add "main1"(type=main_){
get_keyboard ABCDEFGHIJKLMNOPQRSTUVWXYZ,abcdefghijklmnopqrstuvwxyz,1234567890,$";:!?#[].'"$; //这里的$""$代表只有里面的东西，$"""$只算"
}
function_add "help"(type=normal){
printf("RealEngine shell (ashell.re) v0.2.1\n");
printf("PATH: $BIN\n");
printf("-------------------- builtin --------------------\n");
printf("  help            show this message\n");
printf("  exit            quit the shell\n");
printf("  cd      <dir>   change directory\n");
printf("  pwd             print working directory\n");
printf("  echo    <text>  print text\n");
printf("  exec    <cmd>   run a command via sh -c\n");
printf("  which   <cmd>   locate a command\n");
printf("-------------------- coreutils ------------------\n");
printf("  ls cp mv rm mkdir touch stat du\n");
printf("  cat head grep wc sort uniq cut tr sed awk diff tee seq\n");
printf("  ps kill env date uptime df free hostname\n");
printf("  ping curl wget ip sleep true false\n");
printf("  whoami uname id hostname\n");
}
function_add "builtin"(type=normal){
if $INPUT is "pwd";then;do="printf("$execute_command pwd\n");";end_if;
if $INPUT is "echo";then;do="printf("$INPUT_OPT\n");";end_if;
if $INPUT is "exec";then;do="execve('$BIN_sh', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "sh";then;do="execve('$BIN_sh', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "cd";then;do="execve('$BIN_sh', '-c', 'cd $INPUT_OPT && pwd')";end_if;
if $INPUT is "which";then;do="execve('$BIN_which', '$INPUT_OPT')";end_if;
if $INPUT is "ls";then;do="execve('$BIN_ls', '--color=never', '$INPUT_OPT')";end_if;
if $INPUT is "cp";then;do="execve('$BIN_cp', '-rv', '$INPUT_OPT')";end_if;
if $INPUT is "mv";then;do="execve('$BIN_mv', '-v', '$INPUT_OPT')";end_if;
if $INPUT is "rm";then;do="execve('$BIN_rm', '-rf', '$INPUT_OPT')";end_if;
if $INPUT is "mkdir";then;do="execve('$BIN_mkdir', '-p', '$INPUT_OPT')";end_if;
if $INPUT is "touch";then;do="execve('$BIN_touch', '$INPUT_OPT')";end_if;
if $INPUT is "stat";then;do="execve('$BIN_stat', '$INPUT_OPT')";end_if;
if $INPUT is "du";then;do="execve('$BIN_du', '-sh', '$INPUT_OPT')";end_if;
if $INPUT is "cat";then;do="execve('$BIN_cat', '$INPUT_OPT')";end_if;
if $INPUT is "head";then;do="execve('$BIN_head', '-n', '20', '$INPUT_OPT')";end_if;
if $INPUT is "grep";then;do="execve('$BIN_grep', '--color=never', '$INPUT_OPT')";end_if;
if $INPUT is "wc";then;do="execve('$BIN_wc', '-l', '-w', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "sort";then;do="execve('$BIN_sort', '$INPUT_OPT')";end_if;
if $INPUT is "uniq";then;do="execve('$BIN_uniq', '-c', '$INPUT_OPT')";end_if;
if $INPUT is "cut";then;do="execve('$BIN_cut', '$INPUT_OPT')";end_if;
if $INPUT is "tr";then;do="execve('$BIN_tr', '$INPUT_OPT')";end_if;
if $INPUT is "sed";then;do="execve('$BIN_sed', '$INPUT_OPT')";end_if;
if $INPUT is "awk";then;do="execve('$BIN_awk', '$INPUT_OPT')";end_if;
if $INPUT is "diff";then;do="execve('$BIN_diff', '-u', '$INPUT_OPT')";end_if;
if $INPUT is "tee";then;do="execve('$BIN_tee', '$INPUT_OPT')";end_if;
if $INPUT is "seq";then;do="execve('$BIN_seq', '$INPUT_OPT')";end_if;
if $INPUT is "ps";then;do="execve('$BIN_ps', 'aux')";end_if;
if $INPUT is "kill";then;do="execve('$BIN_kill', '$INPUT_OPT')";end_if;
if $INPUT is "env";then;do="execve('$BIN_env')";end_if;
if $INPUT is "date";then;do="execve('$BIN_date', '$INPUT_OPT')";end_if;
if $INPUT is "uptime";then;do="execve('$BIN_uptime')";end_if;
if $INPUT is "df";then;do="execve('$BIN_df', '-h')";end_if;
if $INPUT is "free";then;do="execve('$BIN_free', '-h')";end_if;
if $INPUT is "hostname";then;do="execve('$BIN_hostname')";end_if;
if $INPUT is "ping";then;do="execve('$BIN_ping', '-c', '4', '$INPUT_OPT')";end_if;
if $INPUT is "curl";then;do="execve('$BIN_curl', '-sSL', '$INPUT_OPT')";end_if;
if $INPUT is "wget";then;do="execve('$BIN_wget', '-qO-', '$INPUT_OPT')";end_if;
if $INPUT is "ip";then;do="execve('$BIN_ip', 'addr')";end_if;
if $INPUT is "sleep";then;do="execve('$BIN_sleep', '$INPUT_OPT')";end_if;
if $INPUT is "true";then;do="execve('$BIN_true')";end_if;
if $INPUT is "false";then;do="execve('$BIN_false')";end_if;
if $INPUT is "whoami";then;do="execve('$BIN_whoami')";end_if;
if $INPUT is "uname";then;do="execve('$BIN_uname', '-a')";end_if;
if $INPUT is "id";then;do="execve('$BIN_id')";end_if;
}
function_add "main"(type=main){
printf("$execute_command $BIN_whoami");
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