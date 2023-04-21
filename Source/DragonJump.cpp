// by Dmitry Kolontay

#include "DragonJumpFramework.h"
#include <string>
#include <Windows.h>
#include <regex>

int main(int argc, char* argv[])
{
	int sizeX{ 0 };
	int sizeY{ 0 };
	if (argc > 1) {
		const std::vector<std::string> args(argv + 1, argv + argc);
		const std::regex sizeStr{ "^\\d{3,5}[^( |\\d)]\\d{3,5}$" };
		for (auto i{ 0 }; i < args.size() - 1; ++i) {
			if (args.at(i) == "-window" && std::regex_match(args.at(i + 1), sizeStr)) {
				const std::regex substr{ "[^( |\\d)]\\d{3,}?$" };
				std::smatch match;
				std::regex_search(args.at(i + 1), match, substr);
				sizeX = std::stoi(match.prefix().str());
				sizeY = std::stoi(match[0].str().substr(1));
				break;
			}
		}
	}
	DragonJumpFramework* framework{ nullptr };
	if (sizeX == 0 || sizeY == 0) 
		framework = new DragonJumpFramework{ true };
	else 
		framework = new DragonJumpFramework{ true, sizeX, sizeY };
	run(framework);
	return 0;
}
