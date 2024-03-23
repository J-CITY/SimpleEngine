#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

namespace IKIGAI::UTILS::LOGG {
	class Logger;

	class Log {
	public:
		friend class Logger;
		enum class Level {
			Debug = 0,
			Info,
			Notice,
			Warning,
			Error,
			Critic
		};
	
	public:
		template <typename T>
		Log& operator<< (const T& aValue) {
			if (nullptr != mpStream) {
				(*mpStream) << aValue;
			}
			return (*this);
		}
	
		~Log();
	
		[[nodiscard]] inline Level getLevel() const {
			return mLevel;
		}

		inline const std::ostringstream& getStream() const {
			return *mpStream;
		}
		
		static std::string ToString(Log::Level aLevel);
		static Level ToLevel(const std::string& apLevel);
	
	private:
		Log(const Logger& aLogger, Level aSeverity);

		//Log(const Log&);
		//void operator=(const Log&);
	
	private:
		const Logger& mLogger;
		Level mLevel;
		std::ostringstream* mpStream;
	};
	

	class Channel {
	public:
		using Ptr = std::shared_ptr<Channel>;
		typedef std::map<std::string, Ptr> Map;
	public:
		Channel(const std::string& apChannelName, Log::Level aChannelLevel) :
			mName(apChannelName),
			mLevel(aChannelLevel) {}
		
		~Channel() = default;
		
		inline const std::string& getName() const {
			return mName;
		}
		
		inline void setLevel(Log::Level aLevel) {
			mLevel = aLevel;
		}
		
		inline Log::Level getLevel() const {
			return mLevel;
		}

	private:
		Channel(Channel&);
		void operator=(Channel&);
	private:
		std::string mName;
		Log::Level mLevel;
	};

	class Output {
	public:
		using Ptr = std::shared_ptr<Output>;
		virtual ~Output() = default;
		virtual void output(const Channel::Ptr& aChannelPtr, const Log& aLog) const = 0;
		[[nodiscard]] virtual const std::string& name() const = 0;
	};

	class Modifier {
	public:
		enum class Code {
			FG_BLACK = 30,
			FG_RED = 31,
			FG_YELLOW = 33,
			FG_GREEN = 32,
			FG_BLUE = 34,
			FG_MAGENTA = 35,
			FG_CYAN = 36,
			FG_WHITE = 37,
			FG_DEFAULT = 39,
			BG_BLACK = 40,
			BG_RED = 41,
			BG_YELLOW = 43,
			BG_GREEN = 42,
			BG_BLUE = 44,
			BG_MAGENTA = 45,
			BG_CYAN = 46,
			BG_WHITE = 47,
			BG_DEFAULT = 49
		};
		Modifier(Code pCode) : code(pCode) {}
		friend std::ostream& operator<<(std::ostream& os, const Modifier& mod) {
			return os << "\033[" << static_cast<int>(mod.code) << "m";
		}
	private:
		Code code;
	};

	class OutputConsole : public Output {
	public:
		explicit OutputConsole() = default;
		~OutputConsole() override = default;
		[[nodiscard]] const std::string& name() const override;
		void output(const Channel::Ptr& aChannelPtr, const Log& aLog) const override;
	};

	
	 class Logger {
	 public:
		friend Log;
		explicit Logger(const std::string& channelName = "default");
	
		~Logger();

		Log debug() const;
		Log info() const;
		Log notice() const;
		Log warning() const;
		Log error() const;
		Log critic() const;

		inline const std::string& getName(void) const {
			return mChannelPtr->getName();
		}

		inline void setLevel(Log::Level aLevel) {
			mChannelPtr->setLevel(aLevel);
		}
	
		inline Log::Level getLevel(void) const {
			return mChannelPtr->getLevel();
		}
	private:
		void output(const Log& aLog) const;
		Channel::Ptr  mChannelPtr;
	};

	struct Manager {
	public:
		static void ClearOutputs();

		static Channel::Ptr Get(const std::string& apChannelName);
		static void Output(const Channel::Ptr& aChannelPtr, const Log& aLog);
		inline static void SetDefaultLevel(Log::Level aLevel) {
			mDefaultLevel = aLevel;
		}
		inline static void AddOutput(Output::Ptr out) {
			mOutputList.push_back(out);
		}
	private:
		inline static Channel::Map mChannelMap;
		inline static std::vector<Output::Ptr> mOutputList;
		inline static Log::Level mDefaultLevel = Log::Level::Critic;
	};
}

