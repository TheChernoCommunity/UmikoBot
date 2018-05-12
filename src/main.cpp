#include "UmikoBot.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	UmikoBot     bot;

	// Retrieve token from program arguments
	QStringList arguments = app.arguments();
	if (arguments.count() < 2)
	{
		QMessageBox::critical(nullptr, "No token", "No token was provided!");
		return -1;
	}

	// Log in
	Discord::Token token;
	token.generate(arguments.last(), Discord::Token::Type::BOT);
	bot.login(token);

	return app.exec();
}
