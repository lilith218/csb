/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#include <hiredis/hiredis.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#define SLEEP_MICROSEC 10U
#define PRINT_FREQ     10000U

typedef enum { READ, WRITE } CMD;

void
process(redisReply *reply)
{
    if (reply == NULL) {
        printf("Null reply\n");
        return;
    }
    switch (reply->type) {
        case REDIS_REPLY_NIL:
            printf("Nil reply\n");
            break;
        case REDIS_REPLY_STRING:
            printf("String: %s len(%zu)\n", reply->str, reply->len);
            break;
        case REDIS_REPLY_STATUS:
            printf("Status: %s\n", reply->str);
            break;
        case REDIS_REPLY_INTEGER:
            printf("Integer: %lld\n", reply->integer);
            break;
        case REDIS_REPLY_ARRAY:
            printf("Array of %zu elements\n", reply->elements);
            for (size_t i = 0; i < reply->elements; i++) {
                process(reply->element[i]);
            }
            break;
        case REDIS_REPLY_ERROR:
            printf("Error: %s\n", reply->str);
            break;
        default:
            printf("Unknown reply type: %d\n", reply->type);
    }
}

bool
communicate(CMD cmd, redisContext *ctx)
{
    const char *msg   = (cmd == READ) ? "GET foo" : "SET foo bar";
    redisReply *reply = redisCommand(ctx, msg);
    if (reply == NULL) {
        printf("Null reply (connection error)\n");
        return false;
    }
    if (ctx->err) {
        printf("Response Error: %d => %s\n", ctx->err, ctx->errstr);
        freeReplyObject(reply);
        return false;
    }
    process(reply);
    freeReplyObject(reply);
    return true;
}

/* ./client <ip> <port> */
int
main(int argc, char *argv[])
{
    int attempt = 0;
    assert(argc == 3);

    char *ip = argv[1];
    int port = atoi(argv[2]);

    printf("Connecting to Redis server on %s:%d ...\n", ip, port);
    redisContext *ctx = NULL;
    do {
        ctx = redisConnect(ip, port);
        if (ctx == NULL || ctx->err) {
            if (ctx && ctx->err) {
                if ((attempt++ % PRINT_FREQ == 0)) {
                    printf("Connection error: %s\n", ctx->errstr);
                }
                redisFree(ctx);
            }
            usleep(SLEEP_MICROSEC);
            continue;
        } else {
            printf("Connection succeeded\n");
            break;
        }
    } while (true);

    // send messages non-stop
    bool success = true;

    while (success) {
        success = communicate(WRITE, ctx); // SET foo bar
        success &= communicate(READ, ctx); // GET foo
    }

    printf("Finished sending messages. last result %s\n",
           success ? "success" : "failure");
    redisFree(ctx);
    return 0;
}
