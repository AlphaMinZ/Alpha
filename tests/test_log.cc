#include "../alpha/log.h"

#include <unistd.h>

alphaMin::Logger::ptr g_logger(new alphaMin::Logger);

int main(int argc, char** argv) {
    g_logger->addAppender(alphaMin::LogAppender::ptr(new alphaMin::StdoutLogAppender));
    ALPHA_LOG_FATAL(g_logger) << "fatal msg";
    ALPHA_LOG_ERROR(g_logger) << "err msg";
    ALPHA_LOG_INFO(g_logger) << "info msg";
    ALPHA_LOG_DEBUG(g_logger) << "debug msg";

    ALPHA_LOG_INFO(g_logger) << "==========================================";

}