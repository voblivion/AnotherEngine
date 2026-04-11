#include <filesystem>
#include <fstream>
#include <iostream>


// Only supports filtering out:
// - #ifdef __cplusplus with #else (no #if defined or #elif)
// - #ifndef __cplusplus with #else (no #if !defined or #elif)
bool generate(std::ifstream& a_inputFile, std::ofstream& a_outputFile)
{
	auto isGlslOnly = false;
	auto insideGlslOnlyScopeSkip = 0;
	auto isCppOnly = false;
	auto insideCppOnlyScopeSkip = 0;

	std::string line;
	while (std::getline(a_inputFile, line))
	{
		if (line.starts_with("#ifndef __cplusplus"))
		{
			if (isCppOnly)
			{
				return false;
			}
			isGlslOnly = true;
		}
		else if (line.starts_with("#ifdef __cplusplus"))
		{
			if (isGlslOnly)
			{
				return false;
			}
			isCppOnly = true;
		}
		else if (line.starts_with("#if"))
		{
			if (isCppOnly)
			{
				++insideCppOnlyScopeSkip;
			}
			else if (isGlslOnly)
			{
				++insideGlslOnlyScopeSkip;
				a_outputFile << line << '\n';
			}
			else
			{
				a_outputFile << line << '\n';
			}
		}
		else if (line.starts_with("#else"))
		{
			if (isCppOnly)
			{
				if (insideCppOnlyScopeSkip == 0)
				{
					isCppOnly = false;
					isGlslOnly = true;
				}
			}
			else if (isGlslOnly)
			{
				if (insideGlslOnlyScopeSkip == 0)
				{
					isGlslOnly = false;
					isCppOnly = true;
				}
				else
				{
					a_outputFile << line << '\n';
				}
			}
			else
			{
				a_outputFile << line << '\n';
			}
		}
		else if (line.starts_with("#endif"))
		{
			if (isCppOnly)
			{
				if (insideCppOnlyScopeSkip == 0)
				{
					isCppOnly = false;
				}
				else
				{
					--insideCppOnlyScopeSkip;
				}
			}
			else if (isGlslOnly)
			{
				if (insideGlslOnlyScopeSkip == 0)
				{
					isGlslOnly = false;
				}
				else
				{
					--insideGlslOnlyScopeSkip;
					a_outputFile << line << '\n';
				}
			}
			else
			{
				a_outputFile << line << '\n';
			}
		}
		else
		{
			if (!isCppOnly)
			{
				a_outputFile << line << '\n';
			}
		}
	}

	return true;
}

int main(int a_argc, char** a_argv)
{
	if (a_argc < 3)
	{
		std::cerr << "Must provide Source and Target directories." << std::endl;
		return EXIT_FAILURE;
	}

	auto const sourceDir = std::filesystem::path(a_argv[1]);
	if (!std::filesystem::exists(sourceDir))
	{
		std::cerr << "Invalid Source directory: " << sourceDir << std::endl;
		return EXIT_FAILURE;
	}

	auto const targetDir = std::filesystem::path(a_argv[2]);
	std::filesystem::create_directories(targetDir);

	for (auto const& entry : std::filesystem::recursive_directory_iterator(sourceDir))
	{
		auto const sourcePath = entry.path();
		auto const sourcePathRelative = sourcePath.lexically_relative(sourceDir);
		if (entry.is_directory())
		{
			std::filesystem::create_directories(targetDir / sourcePathRelative);
		}
		else
		{
			auto sourceFile = std::ifstream(sourcePath);
			if (!sourceFile.is_open())
			{
				std::cerr << "Failed to open `" << sourcePath << "` for read." << std::endl;
				continue;
			}

			auto targetPath = targetDir / sourcePathRelative;
			targetPath.replace_extension(".glsl");
			auto targetFile = std::ofstream(targetPath);
			if (!targetFile.is_open())
			{
				std::cerr << "Failed to open " << targetPath << " for write." << std::endl;
				continue;
			}

			if (generate(sourceFile, targetFile))
			{
				std::cout << "Successfully generated " << targetPath << "." << std::endl;
			}
			else
			{
				std::cerr << "File " << sourcePath << " cannot be converted." << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}
