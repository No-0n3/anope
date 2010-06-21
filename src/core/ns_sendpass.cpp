/* NickServ core functions
 *
 * (C) 2003-2010 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 *
 */
/*************************************************************************/

#include "module.h"

static bool SendPassMail(User *u, NickAlias *na, const std::string &pass);

class CommandNSSendPass : public Command
{
 public:
	CommandNSSendPass() : Command("SENDPASS", 1, 1)
	{
	}

	CommandReturn Execute(User *u, const std::vector<ci::string> &params)
	{
		const char *nick = params[0].c_str();
		NickAlias *na;

		if (Config.RestrictMail && !u->Account()->HasCommand("nickserv/sendpass"))
			notice_lang(Config.s_NickServ, u, ACCESS_DENIED);
		else if (!(na = findnick(nick)))
			notice_lang(Config.s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
		else if (na->HasFlag(NS_FORBIDDEN))
			notice_lang(Config.s_NickServ, u, NICK_X_FORBIDDEN, na->nick);
		else
		{
			std::string tmp_pass;
			if (enc_decrypt(na->nc->pass,tmp_pass) == 1)
			{
				if (SendPassMail(u, na, tmp_pass))
				{
					Alog() << Config.s_NickServ << ": " << u->GetMask() << " used SENDPASS on " << nick;
					notice_lang(Config.s_NickServ, u, NICK_SENDPASS_OK, nick);
				}
			}
			else
				notice_lang(Config.s_NickServ, u, NICK_SENDPASS_UNAVAILABLE);
		}

		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		notice_help(Config.s_NickServ, u, NICK_HELP_SENDPASS);
		return true;
	}

	void OnSyntaxError(User *u, const ci::string &subcommand)
	{
		syntax_error(Config.s_NickServ, u, "SENDPASS", NICK_SENDPASS_SYNTAX);
	}

	void OnServHelp(User *u)
	{
		notice_lang(Config.s_NickServ, u, NICK_HELP_CMD_SENDPASS);
	}
};

class NSSendPass : public Module
{
 public:
	NSSendPass(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion(VERSION_STRING);
		this->SetType(CORE);

		this->AddCommand(NickServ, new CommandNSSendPass());

		if (!Config.UseMail)
			throw ModuleException("Not using mail, whut.");

		std::string tmp_pass = "plain:tmp";
		if (enc_decrypt(tmp_pass, tmp_pass) == -1)
			throw ModuleException("Incompatible with the encryption module being used");
	}
};

static bool SendPassMail(User *u, NickAlias *na, const std::string &pass)
{
	char subject[BUFSIZE], message[BUFSIZE];

	snprintf(subject, sizeof(subject), getstring(na, NICK_SENDPASS_SUBJECT), na->nick);
	snprintf(message, sizeof(message), getstring(na, NICK_SENDPASS), na->nick, pass.c_str(), Config.NetworkName);

	return Mail(u, na->nc, Config.s_NickServ, subject, message);
}

MODULE_INIT(NSSendPass)