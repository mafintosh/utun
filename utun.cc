#include <uv.h>
#include <nan.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/kern_event.h>
#include <sys/socket.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <ctype.h>
#include <fcntl.h>

using namespace v8;

#define UTUN_CONTROL_NAME "com.apple.net.utun_control"
#define UTUN_OPT_IFNAME 2

static void poll_worker (uv_poll_t *req, int status, int events) {
  Nan::HandleScope scope;
  Nan::Callback *callback = (Nan::Callback*) req->data;
  callback->Call(0, 0);
}

static int open_tun_socket () {
  struct sockaddr_ctl addr;
  struct ctl_info info;
  char ifname[10];
  socklen_t ifname_len = sizeof(ifname);
  int fd = -1;
  int err = 0;

  fd = socket (PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
  if (fd < 0) return fd;

  bzero(&info, sizeof (info));
  strncpy(info.ctl_name, UTUN_CONTROL_NAME, MAX_KCTL_NAME);

  err = ioctl(fd, CTLIOCGINFO, &info);
  if (err != 0) goto on_error;

  addr.sc_len = sizeof(addr);
  addr.sc_family = AF_SYSTEM;
  addr.ss_sysaddr = AF_SYS_CONTROL;
  addr.sc_id = info.ctl_id;
  addr.sc_unit = 0;

  err = connect(fd, (struct sockaddr *)&addr, sizeof (addr));
  if (err != 0) goto on_error;

  // TODO: forward ifname (we just expect it to be utun0 for now...)
  err = getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifname, &ifname_len);
  if (err != 0) goto on_error;

  err = fcntl(fd, F_SETFL, O_NONBLOCK);
  if (err != 0) goto on_error;

  fcntl(fd, F_SETFD, FD_CLOEXEC);
  if (err != 0) goto on_error;

on_error:
  if (err != 0) {
    close(fd);
    return err;
  }

  return fd;
}

NAN_METHOD(Utun) {
  int fd = open_tun_socket();
  if (fd < 0) {
    Nan::ThrowError("Could not open tun device");
    return;
  }
  uv_poll_t *handle = (uv_poll_t *) malloc(sizeof(uv_poll_t));
  Nan::Callback *callback = new Nan::Callback(info[0].As<Function>());

  handle->data = callback;
  uv_poll_init(uv_default_loop(), handle, fd);
  uv_poll_start(handle, UV_READABLE, poll_worker);

  info.GetReturnValue().Set(fd);
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New<String>("utun").ToLocalChecked(), Nan::New<FunctionTemplate>(Utun)->GetFunction());
}

NODE_MODULE(utun, Init)
