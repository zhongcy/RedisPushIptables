/*
 *      Author: Gandalf
 *      email： zhibu1991@gmail.com
 */
#include "redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int redis_waitpid(pid_t pid)
{
	int             rc, status;
	do
	{
		if (-1 == (rc = waitpid(pid, &status, WUNTRACED)))
		{
			goto exit;
		}
	}
	while (!WIFEXITED(status) && !WIFSIGNALED(status));
exit:

	return rc;
}

int execute_fork()
{
	fflush(stdout);
	fflush(stderr);
	return fork();
}

int  execute_popen(pid_t *pid, const char *command)
{
	int             fd[2];

	if (-1 == pipe(fd))
		return -1;

	if (-1 == (*pid = execute_fork()))
	{
		close(fd[0]);
		close(fd[1]);
		return -1;
	}

	if (0 != *pid)  /* parent process */
	{
		close(fd[1]);
		return fd[0];
	}

	close(fd[0]);
	dup2(fd[1], STDOUT_FILENO);
	dup2(fd[1], STDERR_FILENO);
	close(fd[1]);
	if (-1 == setpgid(0, 0))
	{
		exit(EXIT_SUCCESS);
	}

	execl("/bin/sh", "sh", "-c", command, NULL);
	exit(EXIT_SUCCESS);
}

int DROP_Insert_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
	if (argc != 2)
		return RedisModule_WrongArity(ctx);

	RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1],
	REDISMODULE_READ | REDISMODULE_WRITE);
	pid_t pid;
	int fd;
	char tmp_buf[4096];

	static char check_command[256], insert_command[256];
	sprintf(check_command, "iptables -C INPUT -s %s -j DROP",
			RedisModule_StringPtrLen(argv[1], NULL));
	sprintf(insert_command, "iptables -I INPUT -s %s -j DROP",
			RedisModule_StringPtrLen(argv[1], NULL));
	printf("%s || %s\n", RedisModule_StringPtrLen(argv[0], NULL),
			RedisModule_StringPtrLen(argv[1], NULL));
	fd = execute_popen(&pid, check_command);
	redis_waitpid(pid);
	if (0 < read(fd, tmp_buf, sizeof(tmp_buf) - 1)) {
		close(fd);
		execute_popen(&pid, insert_command);
		redis_waitpid(pid);
	}
	close(fd);

	RedisModule_StringSet(key, argv[1]);
	size_t newlen = RedisModule_ValueLength(key);
	RedisModule_CloseKey(key);
	RedisModule_ReplyWithLongLong(ctx, newlen);
	return REDISMODULE_OK;
}

int DROP_Delete_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
	if (argc != 2)
		return RedisModule_WrongArity(ctx);

	RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1],
	REDISMODULE_READ | REDISMODULE_WRITE);
	pid_t pid;
	int fd;
	static char insert_command[256];

	sprintf(insert_command, "iptables -D INPUT -s %s -j DROP",
			RedisModule_StringPtrLen(argv[1], NULL));
	printf("%s || %s\n", RedisModule_StringPtrLen(argv[0], NULL),
			RedisModule_StringPtrLen(argv[1], NULL));

	fd = execute_popen(&pid, insert_command);
	redis_waitpid(pid);
	close(fd);

	RedisModule_DeleteKey(key);
	size_t newlen = RedisModule_ValueLength(key);
	RedisModule_CloseKey(key);
	RedisModule_ReplyWithLongLong(ctx, newlen);
	return REDISMODULE_OK;
}
int ACCEPT_Insert_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
	if (argc != 2)
		return RedisModule_WrongArity(ctx);

	RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1],
	REDISMODULE_READ | REDISMODULE_WRITE);
	pid_t pid;
	int fd;
	char tmp_buf[4096];

	static char check_command[256], insert_command[256];
	sprintf(check_command, "iptables -C INPUT -s %s -j ACCEPT",
			RedisModule_StringPtrLen(argv[1], NULL));
	sprintf(insert_command, "iptables -I INPUT -s %s -j ACCEPT",
			RedisModule_StringPtrLen(argv[1], NULL));
	printf("%s || %s\n", RedisModule_StringPtrLen(argv[0], NULL),
			RedisModule_StringPtrLen(argv[1], NULL));
	fd = execute_popen(&pid, check_command);
	redis_waitpid(pid);
	if (0 < read(fd, tmp_buf, sizeof(tmp_buf) - 1)) {
		close(fd);
		execute_popen(&pid, insert_command);
		redis_waitpid(pid);
	}
	close(fd);

	RedisModule_StringSet(key, argv[1]);

	size_t newlen = RedisModule_ValueLength(key);
	RedisModule_CloseKey(key);
	RedisModule_ReplyWithLongLong(ctx, newlen);
	return REDISMODULE_OK;
}
int ACCEPT_Delete_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
	if (argc != 2)
		return RedisModule_WrongArity(ctx);

	RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1],
	REDISMODULE_READ | REDISMODULE_WRITE);
	pid_t pid;
	int fd;
	static char insert_command[256];

	sprintf(insert_command, "iptables -D INPUT -s %s -j ACCEPT",
			RedisModule_StringPtrLen(argv[1], NULL));
	printf("%s || %s\n", RedisModule_StringPtrLen(argv[0], NULL),
			RedisModule_StringPtrLen(argv[1], NULL));

	fd = execute_popen(&pid, insert_command);
	redis_waitpid(pid);
	close(fd);

	RedisModule_DeleteKey(key);
	size_t newlen = RedisModule_ValueLength(key);
	RedisModule_CloseKey(key);
	RedisModule_ReplyWithLongLong(ctx, newlen);
	return REDISMODULE_OK;
}

int TTL_DROP_Insert_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
	if (argc != 3)
				return RedisModule_WrongArity(ctx);
			RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1],
			REDISMODULE_READ | REDISMODULE_WRITE);
			long long count;
			    if ((RedisModule_StringToLongLong(argv[2],&count) != REDISMODULE_OK) ||
			        (count < 0)) {
			        return RedisModule_ReplyWithError(ctx,"ERR invalid count");
			    }
			pid_t pid;
			int fd;
			char tmp_buf[4096];

			static char check_command[256], insert_command[256];
			sprintf(check_command, "iptables -C INPUT -s %s -j DROP",
					RedisModule_StringPtrLen(argv[1], NULL));
			sprintf(insert_command, "iptables -I INPUT -s %s -j DROP",
					RedisModule_StringPtrLen(argv[1], NULL));
			printf("%s || %s\n", RedisModule_StringPtrLen(argv[0], NULL),
					RedisModule_StringPtrLen(argv[1], NULL));
			fd = execute_popen(&pid, check_command);
			redis_waitpid(pid);
			if (0 < read(fd, tmp_buf, sizeof(tmp_buf) - 1)) {
				close(fd);
				execute_popen(&pid, insert_command);
				redis_waitpid(pid);
			}
			close(fd);
			RedisModule_StringSet(key, argv[1]);
			   RedisModule_SetExpire(key, count*1000);
				size_t newlen = RedisModule_ValueLength(key);
				RedisModule_CloseKey(key);
				RedisModule_ReplyWithLongLong(ctx, newlen);
			return REDISMODULE_OK;
}

/* This function must be present on each Redis module. It is used in order to
 * register the commands into the Redis server. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if (RedisModule_Init(ctx,"iptables-input-filter",1,REDISMODULE_APIVER_1)
			== REDISMODULE_ERR) return REDISMODULE_ERR;

	/* Log the list of parameters passing loading the module. */
	for (int j = 0; j < argc; j++) {
		const char *s = RedisModule_StringPtrLen(argv[j],NULL);
		printf("Module loaded with ARGV[%d] = %s\n", j, s);
	}

	if (RedisModule_CreateCommand(ctx, "drop.insert",
			DROP_Insert_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;
	if (RedisModule_CreateCommand(ctx, "drop.delete",
			DROP_Delete_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;
	if (RedisModule_CreateCommand(ctx, "accept.insert",
			ACCEPT_Insert_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;
	if (RedisModule_CreateCommand(ctx, "accept.delete",
			ACCEPT_Delete_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;
	if (RedisModule_CreateCommand(ctx, "ttl.drop.insert",
			TTL_DROP_Insert_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
			return REDISMODULE_ERR;

	return REDISMODULE_OK;
}
