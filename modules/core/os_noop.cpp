/* OperServ core functions
 *
 * (C) 2003-2011 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandOSNOOP : public Command
{
 public:
	CommandOSNOOP() : Command("NOOP", 2, 2, "operserv/noop")
	{
	}

	CommandReturn Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &cmd = params[0];
		const Anope::string &server = params[1];

		if (cmd.equals_ci("SET"))
		{
			Anope::string reason;

			/* Remove the O:lines */
			ircdproto->SendSVSNOOP(server, 1);

			reason = "NOOP command used by " + u->nick;
			if (Config->WallOSNoOp)
				ircdproto->SendGlobops(OperServ, "\2%s\2 used NOOP on \2%s\2", u->nick.c_str(), server.c_str());
			source.Reply(OPER_NOOP_SET, server.c_str());

			/* Kill all the IRCops of the server */
			patricia_tree<User *, ci::ci_char_traits>::iterator it(UserListByNick);
			for (bool next = it.next(); next;)
			{
				User *u2 = *it;
				next = it.next();

				if (u2 && u2->HasMode(UMODE_OPER) && Anope::Match(u2->server->GetName(), server, true))
					kill_user(Config->s_OperServ, u2, reason);
			}
		}
		else if (cmd.equals_ci("REVOKE"))
		{
			ircdproto->SendSVSNOOP(server, 0);
			source.Reply(OPER_NOOP_REVOKE, server.c_str());
		}
		else
			this->OnSyntaxError(source, "");
		return MOD_CONT;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		source.Reply(OPER_HELP_NOOP);
		return true;
	}

	void OnSyntaxError(CommandSource &source, const Anope::string &subcommand)
	{
		SyntaxError(source, "NOOP", OPER_NOOP_SYNTAX);
	}

	void OnServHelp(CommandSource &source)
	{
		source.Reply(OPER_HELP_CMD_NOOP);
	}
};

class OSNOOP : public Module
{
	CommandOSNOOP commandosnoop;

 public:
	OSNOOP(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetType(CORE);

		this->AddCommand(OperServ, &commandosnoop);
	}
};

MODULE_INIT(OSNOOP)
