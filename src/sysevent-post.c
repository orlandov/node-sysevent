#include <stdio.h>
#include <libsysevent.h>
#include <libnvpair.h>

int main(int argc, char **argv) {
  nvlist_t *attr_list;
  uint32_t uint32_val = 0XFFFFFFFF;
  char     *string_val = "string value data";
  int err;
  sysevent_id_t eid;

  if (nvlist_alloc(&attr_list, 0, 0) == 0) {
    err = nvlist_add_uint32(attr_list, "uint32 data", uint32_val);
    if (err == 0)
      err = nvlist_add_string(attr_list, "str data",
          string_val);
    if (err == 0)
      err = sysevent_post_event("class1", "ESC_MYSUBCLASS",
          "SUNW", argv[0], NULL, &eid);
    if (err != 0)
      fprintf(stderr, "error logging system event\n");
    nvlist_free(attr_list);
  }

  return 0;
}
