#include <opium/opium.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

typedef struct {
  OpiHeader header;
  socklen_t addrlen;
  char data[];
} SockAddr;
#define SOCK_ADDR(x) ((SockAddr*)(x))

static
opi_type_t sock_addr_type;

OPI_DEF(getprotobyname_,
  opi_arg(name, opi_str_type)
  struct protoent *proto = getprotobyname(OPI_STR(name)->str);
  if (proto)
    opi_return(opi_num_new(proto->p_proto));
  else
    opi_throw("protocol-error");
)

OPI_DEF(socket_,
  opi_arg(domain, opi_num_type)
  opi_arg(type, opi_num_type)
  opi_arg(protocol, opi_num_type)
  int fd =
    socket(OPI_NUM(domain)->val, OPI_NUM(type)->val, OPI_NUM(protocol)->val);
  if (fd < 0) {
    opi_return(opi_str_new(strerror(errno)));
  } else {
    opi_return(opi_file(fdopen(fd, "r+b"), fclose));
  }
)

static opi_t
inet_aton_(void)
{
  OPI_BEGIN_FN()
  opi_arg(host, opi_str_type)
  struct in_addr addr;
  if (!inet_aton(OPI_STR(host)->str, &addr))
    opi_throw("invalid-host");
  opi_return(opi_num_new(addr.s_addr));
}

OPI_DEF(htons_,
  opi_arg(x, opi_num_type)
  opi_return(opi_num_new(htons(OPI_NUM(x)->val)));
)

OPI_DEF(sockaddr_in_,
  opi_arg(family, opi_num_type)
  opi_arg(port, opi_num_type)
  opi_arg(host, opi_num_type)
  SockAddr *addr = malloc(sizeof(SockAddr) + sizeof(struct sockaddr_in));
  struct sockaddr_in *sa = (void*)addr->data;
  memset(sa, 0, sizeof(struct sockaddr_in));
  sa->sin_family = OPI_NUM(family)->val;
  sa->sin_port = OPI_NUM(port)->val;
  sa->sin_addr.s_addr = OPI_NUM(host)->val;
  addr->addrlen = sizeof(struct sockaddr_in);
  opi_init_cell(addr, sock_addr_type);
  opi_return(OPI(addr));
)

OPI_DEF(connect_,
  opi_arg(sock, opi_file_type)
  opi_arg(addr, sock_addr_type)
  int sockfd = fileno(opi_file_get_value(sock));
  if (sockfd < 0)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  if (connect(sockfd, (void*)SOCK_ADDR(addr)->data, SOCK_ADDR(addr)->addrlen) < 0)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
)

OPI_DEF(gethostbyname_,
  opi_arg(name, opi_str_type)
  struct hostent *he = gethostbyname(OPI_STR(name)->str);
  if (he) {
    opi_t name = opi_str_new(he->h_name);
    opi_t aliases = opi_array_new_empty(1);
    for (int i = 0; he->h_aliases[i]; ++i)
      opi_array_push(aliases, opi_str_new(he->h_aliases[i]));
    opi_t addrtype = opi_num_new(he->h_addrtype);
    opi_t addrs = opi_array_new_empty(1);
    assert(he->h_length == 4);
    for (int i = 0; he->h_addr_list[i]; ++i)
      opi_array_push(addrs, opi_num_new(*(in_addr_t*)(he->h_addr_list[i])));
    opi_t ret = opi_table(opi_nil, 0);
    opi_table_insert(ret, opi_cons(opi_symbol("name"), name), 0, 0);
    opi_table_insert(ret, opi_cons(opi_symbol("aliases"), aliases), 0, 0);
    opi_table_insert(ret, opi_cons(opi_symbol("addrtype"), addrtype), 0, 0);
    opi_table_insert(ret, opi_cons(opi_symbol("addrs"), addrs), 0, 0);
    opi_return(ret);
  } else {
    opi_return(opi_undefined(opi_str_new(hstrerror(h_errno))));
  }
)

int
opium_library(OpiBuilder *bldr)
{
  sock_addr_type = opi_type_new("SockAddr");
  opi_type_set_delete_cell(sock_addr_type, OPI_FREE_CELL);
  opi_builder_def_type(bldr, "SockAddr", sock_addr_type);

  opi_builder_def_const(bldr, "afUnix", opi_num_new(AF_UNIX));
  opi_builder_def_const(bldr, "afLocal", opi_num_new(AF_LOCAL));
  opi_builder_def_const(bldr, "afInet", opi_num_new(AF_INET));
  opi_builder_def_const(bldr, "afInet6", opi_num_new(AF_INET6));

  opi_builder_def_const(bldr, "sockStream", opi_num_new(SOCK_STREAM));

  opi_builder_def_const(bldr, "getprotobyname", opi_fn_new(getprotobyname_, 1));
  opi_builder_def_const(bldr, "socket", opi_fn_new(socket_, 3));
  opi_builder_def_const(bldr, "inet_aton", opi_fn_new(inet_aton_, 1));
  opi_builder_def_const(bldr, "htons", opi_fn_new(htons_, 1));
  opi_builder_def_const(bldr, "sockaddr_in", opi_fn_new(sockaddr_in_, 3));
  opi_builder_def_const(bldr, "connect", opi_fn_new(connect_, 2));
  opi_builder_def_const(bldr, "gethostbyname", opi_fn_new(gethostbyname_, 1));

  return 0;
}
