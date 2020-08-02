#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#include "lua.h"

// Node in linked list of lua VM handles
// Each script gets its own VM
struct lua_script_node {
	lua_State *lvm;
	struct lua_script_node *next;
};

struct lua_script_node *lua_scripts = NULL;

void push_script(lua_State *lvm) {
	struct lua_script_node *new;
	if (lua_scripts == NULL) {
		lua_scripts = malloc(sizeof(struct lua_script_node));
		new = lua_scripts;
	} else {
		struct lua_script_node *s;
		for (s = lua_scripts; s->next != NULL; s = s->next);
		new = malloc(sizeof(struct lua_script_node));
		s->next = new;
	}
	new->lvm = lvm;
	new->next = NULL;
}

void init_lua() {
	DIR *script_dir = opendir(SCRIPTDIR);
	if (script_dir == NULL) {
		if (errno != ENOTDIR && errno != ENOENT)
			perror("Error loading scripts");
		return;
	}
	
	for (struct dirent *d = readdir(script_dir); d != NULL; d = readdir(script_dir)) {
		lua_State *lvm = lua_open();
		luaL_openlibs(lvm);
		push_script(lvm);
	}
}

void cleanup_lua() {
	for (struct lua_script_node *s = lua_scripts; s->next != NULL; s = s->next) {
		lua_close(s->lvm);
	}
}
