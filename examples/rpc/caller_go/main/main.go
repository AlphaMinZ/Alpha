package main

import (
	__ "caller_go"
	"context"
	"fmt"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func main() {
	// 通过指定地址，建立与 grpc 服务端的连接
	conn, _ := grpc.Dial("localhost:8000", grpc.WithTransportCredentials(insecure.NewCredentials()))
	// ...
	client := __.NewUserServiceRpcClient(conn)
	resp, _ := client.Login(context.Background(), &__.LoginRequest{
		Name: []byte("alpha_go"),
		Pwd:  []byte("883721"),
	})

	fmt.Println(resp)
}
