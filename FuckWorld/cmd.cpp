#include <metahook.h>
#include "cmd.h"

cmd_function_t *Cmd_GetBase(void)
{
	return (cmd_function_t *)gEngfuncs.GetFirstCmdFunctionHandle();
}

cmd_function_t *Cmd_FindCmd(char *cmd_name)
{
	for (cmd_function_t *cmd = Cmd_GetBase(); cmd; cmd = cmd->next)
	{
		if (!strcmp(cmd->name, cmd_name))
			return cmd;
	}

	return NULL;
}

cmd_function_t *Cmd_FindCmdPrev(char *cmd_name)
{
	cmd_function_t *cmd;

	for (cmd = Cmd_GetBase()->next; cmd->next; cmd = cmd->next)
	{
		if (!strcmp(cmd_name, cmd->next->name))
			return cmd;
	}

	return NULL;
}

int Cmd_AddCommand(char *cmd_name, xcommand_t function)
{
	return gEngfuncs.pfnAddCommand(cmd_name, function);
}

xcommand_t Cmd_HookCmd(char *cmd_name, xcommand_t newfuncs)
{
	cmd_function_t *cmd = Cmd_FindCmd(cmd_name);

	if (!cmd)
	{
		Cmd_AddCommand(cmd_name, newfuncs);
		return newfuncs;
	}

	xcommand_t result = cmd->function;
	cmd->function = newfuncs;
	return result;
}

bool Cmd_RemoveCmd(char *cmd_name)
{
	cmd_function_t *cmd = Cmd_FindCmd(cmd_name);

	if (cmd)
	{
		cmd_function_t *prev = Cmd_FindCmdPrev(cmd_name);

		if (prev)
		{
			prev->next = cmd->next;
			return true;
		}
	}

	return false;
}

void Cmd_Dummy_f(void)
{
}

bool Cmd_BlockCmd(char *cmd_name)
{
	cmd_function_t *cmd = Cmd_FindCmd(cmd_name);

	if (cmd)
	{
		cmd_function_t *prev = Cmd_FindCmdPrev(cmd_name);

		if (prev)
		{
			prev->function = Cmd_Dummy_f;
			return true;
		}
	}

	return false;
}