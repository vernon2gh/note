* 从容器中创建一个新镜像，并push to docker hub

```bash
$ docker commit -m "mesg" <CONTAINER ID> <image>:newtag
$ docker rmi <image>:latest
$ docker tag <image>:newtag <image>:latest
$ docker rmi <image>:newtag
$ docker push <image>:latest
```

