#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h> 

int expipe(char *arg[20][128], int pipenum);
void exec(char *args[]);


int main() {
	/* 输入的命令行 */
	char cmd[256];
	/* 命令行拆解成的各部分，以空指针结尾 */
	char *args[128];
	char *argpipe[20][128];
	while (1) {
		/* 提示符 */
		printf("# ");
		fflush(stdin);
		fgets(cmd, 256, stdin);
		/* 清理结尾的换行符 */
		int i;
		for (i = 0; cmd[i] != '\n'; i++)
			;
		cmd[i] = '\0';
		/* 拆解命令行 */
		args[0] = cmd;
		for (i = 0; *args[i]; i++)
			for (args[i + 1] = args[i] + 1; *args[i + 1]; args[i + 1]++)
				if (*args[i + 1] == ' ') {
					*args[i + 1] = '\0';
					for (args[i + 1]++; *args[i + 1] == ' '; args[i + 1]++);//处理多余空格
					break;
				}
		args[i] = NULL;
		/* 清空命令 */
		for (int j = 0; j < 20; j++)
			for (int k = 0; k < 128; k++)
				argpipe[j][k] = NULL;//初始化，停止位置
		/* 没有输入命令 */
		if (!args[0])
			continue;
		//如果输入exit则直接退出 
		if (strcmp(args[0], "exit") == 0)
			return 0;
		int pipenum = 0, flag = 0;//pipenum表示管道数量 ，flag标记位置 
		int k = 0, argsflag = 0;
		for (int j = 0; j < i; j++) {
			if ((strcmp(args[j], "|") == 0) || (j == i - 1 && pipenum != 0)) {
				for (k = 0, argsflag = flag; k + flag < j; k++) {
					argpipe[pipenum][k] = args[argsflag];
					argsflag++;
				}
				pipenum++;
				flag = j + 1;
			}//二维指针数组第一数表示各管道中的指令，第二维以表示管道之间的空格分隔的字符串
		}
		pipenum--;//循环中将管道数多算了一个 ,在这里减去
		if (pipenum>0)
			argpipe[pipenum][k] = args[argsflag];//最后一个指令的最后一个字符串没有在循环中算上 
		if (pipenum < 0)
			exec(args); //没有管道则按原shell执行 
		else
			expipe(argpipe, pipenum);//如果有管道 

	}
}

int expipe(char *arg[20][128], int pipenum) {
	int fd[2];
	int status;
	int save;
	if (pipe(fd) != 0) {
		perror("pipe error:");
		exit(-1);
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork error:");
		exit(-1);
	}
	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		if (pipenum == 1)
			exec(arg[0]);
		if (pipenum > 1)
			expipe(arg, pipenum - 1);

		exit(0);
	}
	else {
		save = dup(STDIN_FILENO);//备份
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		exec(arg[pipenum]);
		//close(fd[0]);
		dup2(save, STDIN_FILENO);//恢复
		waitpid(pid, &status, 0);
	}
}

void exec(char *args[]) {
	char *p, *name, *value;
	int status;
	if (strcmp(args[0], "cd") == 0) {
		if (args[1])
			chdir(args[1]);
	}
	else if (strcmp(args[0], "pwd") == 0) {
		char wd[4096];
		puts(getcwd(wd, 4096));
	}
	else if (strcmp(args[0], "exit") == 0)
		return;

	else if (strcmp(args[0], "export") == 0) {
		int flag = 0;
		for (name = args[1], value = args[1]; *value; value++) {
			if (*value == '=') {
				flag = 1;
				*value = '\0';
				break;
			}
		}
		if (flag == 0) {
			printf("bad assigment\n");
			return;
		}
		value++;
		setenv(name, value, 1);
	}//修改环境变量
	else {
		pid_t pid = fork();
		if (pid < 0) {
			perror("exec fork error:");
			exit(-1);
		}
		else if (pid == 0) {

			execvp(args[0], args);//子进程执行
			return;
		}

		waitpid(pid, &status, 0);
	}
}
