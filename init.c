#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h> 
#include<sys/stat.h>
#include<fcntl.h>
int expipe(char *arg[20][128], int pipenum);
void exec(char *args[]);


int main() {
	/* ����������� */
	char cmd[256];
	/* �����в��ɵĸ����֣��Կ�ָ���β */
	char *args[128];
	char *argpipe[20][128];
	char *rearg[2][128];
	int reflag=0;
	while (1) {
                 reflag=0;
		/* ��ʾ�� */
		printf("# ");
		fflush(stdin);
		fgets(cmd, 256, stdin);
		/* �����β�Ļ��з� */
		int i;
		for (i = 0; cmd[i] != '\n'; i++)
			;
		cmd[i] = '\0';
		/* ��������� */
		args[0] = cmd;
		for (i = 0; *args[i]; i++)
			for (args[i + 1] = args[i] + 1; *args[i + 1]; args[i + 1]++)
				if (*args[i + 1] == ' ') {
					*args[i + 1] = '\0';
					for (args[i + 1]++; *args[i + 1] == ' '; args[i + 1]++);//�������ո�
					break;
				}
		args[i] = NULL;
int p=i;
		int k = 0;
		for (i = 0;i < 2;i++) 
			for (int j = 0;j<128;j++)
				rearg[i][j] = NULL;
		for (int j = 0;args[j];j++) {
			if (strcmp(args[j], ">") == 0) {
				reflag = 1;
				for (i = 0;i < j;i++)
					rearg[0][i] = args[i];

				for (i = j + 1,k=0;args[i];i++,k++)
					rearg[1][k] = args[i];
			}
			else if (strcmp(args[j], ">>") == 0) {
				reflag = 2;
				for (i = 0;i < j;i++)
					rearg[0][i] = args[i];

				for (i = j + 1,k=0;args[i];i++, k++)
					rearg[1][k] = args[i];
			}
			else if (strcmp(args[j], "<") == 0) {
				reflag = 3;
				for (i = 0;i < j;i++)
					rearg[0][i] = args[i];

				for (i = j + 1, k = 0;args[i];i++, k++)
					rearg[1][k] = args[i];
			}
			
		}
		/* ������� */
		for (int j = 0; j < 20; j++)
			for (int k = 0; k < 128; k++)
				argpipe[j][k] = NULL;//��ʼ����ֹͣλ��
		/* û���������� */
		if (!args[0])
			continue;
		//�������exit��ֱ���˳� 
		if (strcmp(args[0], "exit") == 0)
			return 0;
		int pipenum = 0, flag = 0;//pipenum��ʾ�ܵ����� ��flag���λ�� 
		k = 0;
		int argsflag = 0;
		for (int j = 0; j < p; j++) {
			if ((strcmp(args[j], "|") == 0) || (j == p - 1 && pipenum != 0)) {
				for (k = 0, argsflag = flag; k + flag < j; k++) {
					argpipe[pipenum][k] = args[argsflag];
					argsflag++;
				}
				pipenum++;
				flag = j + 1;
			}//��άָ�������һ����ʾ���ܵ��е�ָ��ڶ�ά�Ա�ʾ�ܵ�֮��Ŀո�ָ����ַ���
		}
		pipenum--;//ѭ���н��ܵ���������һ�� ,�������ȥ

		if (pipenum > 0)
			argpipe[pipenum][k] = args[argsflag];//���һ��ָ������һ���ַ���û����ѭ�������� 
		int d;
		if (reflag == 0) {
			
			if (pipenum < 0)
				exec(args); //û�йܵ���ԭshellִ�� 
			else
				expipe(argpipe, pipenum);//����йܵ� 

		}
		
		else if (reflag == 1) {
			d = open(rearg[1][0], O_CREAT | O_RDWR, 0777);
                        int fx = dup(STDOUT_FILENO);
			dup2(d,STDOUT_FILENO);
			exec(rearg[0]);
                        dup2(fx,STDOUT_FILENO);
			close(d);
 
		}
		else if (reflag == 2) {
			d = open(rearg[1][0], O_CREAT | O_RDWR | O_APPEND, 0777);
                        int fx = dup(STDOUT_FILENO);
			dup2(d, STDOUT_FILENO);
			exec(rearg[0]);
                        dup2(fx,STDOUT_FILENO);
			close(d);
		}
		else if (reflag == 3) {
			d = open(rearg[1][0],O_RDWR);
                        int fx = dup(STDIN_FILENO);
			dup2(d, STDIN_FILENO);
			exec(rearg[0]);
                        dup2(fx,STDIN_FILENO);
			close(d);
		}
		
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
		save = dup(STDIN_FILENO);//����
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		exec(arg[pipenum]);
		//close(fd[0]);
		dup2(save, STDIN_FILENO);//�ָ�
		waitpid(pid, &status, 0);//statusΪ�ӽ���״̬����Ҫ���룬�����wait(NULL)�ɲ�����STDIN_FILENO
	}
}

void exec(char *args[]) {
	char *p, *name, *value;//������export��
	int status;int i;
	int k=0,j;
	int *arg[2][128];
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
	}//�޸Ļ�������
	else {
		pid_t pid = fork();
		if (pid < 0) {
			perror("exec fork error:");
			exit(-1);
		}
		else if (pid == 0) {

			execvp(args[0], args);//�ӽ���ִ��
			return;
		}

		waitpid(pid, &status, 0);
	}
}
