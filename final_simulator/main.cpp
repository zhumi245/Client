#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include "simulator.h"

// ---------------------------------------------------------------------------
// final_simulator - car signal simulator (TCP server on port 5000)
//
// Usage:
//   final_simulator            # default port 5000
//   final_simulator --port N   # custom port
//
// Start this first, then launch the dashboard in network mode:
//   dashboard.exe --connect
//
// The simulator prints status to stdout (connections, commands, errors).
// Ctrl+C or closing the console stops it.
// ---------------------------------------------------------------------------

static QTextStream &ts()
{
    static QTextStream s(stdout);
    return s;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("final_simulator"));

    // parse --port (default 5000, per 需求文档 2.1.1)
    quint16 port = 5000;
    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        const QString &a = args[i];
        if ((a == QString::fromLatin1("--port") || a == QString::fromLatin1("-p"))
            && i + 1 < args.size()) {
            bool ok = false;
            int p = args[++i].toInt(&ok);
            if (ok && p > 0 && p < 65536) port = static_cast<quint16>(p);
        } else if (a == QString::fromLatin1("--help") || a == QString::fromLatin1("-h")) {
            ts() << "Usage: final_simulator [--port N]\n"
                 << "  --port N   TCP port to listen on (default 5000)\n"
                 << "  --help     Show this help\n"
                 << "Start dashboard with: dashboard.exe --connect\n" << flush;
            return 0;
        } else if (a.startsWith(QString::fromLatin1("--port="))) {
            bool ok = false;
            int p = a.mid(7).toInt(&ok);
            if (ok && p > 0 && p < 65536) port = static_cast<quint16>(p);
        }
    }

    Simulator sim(port);

    // wire status messages to stdout
    QObject::connect(&sim, &Simulator::statusMessage,
                     [](const QString &msg) {
        ts() << QString::fromLatin1("[sim] ") << msg << "\n" << flush;
    });

    if (!sim.start()) {
        ts() << QString::fromLatin1(
            "[sim] Failed to start. Is another process already using port %1?")
                .arg(port) << "\n" << flush;
        return 1;
    }

    ts() << QString::fromLatin1(
        "[sim] final_simulator running. Press Ctrl+C to stop.\n"
        "[sim] Dashboard clients should connect to 127.0.0.1:%1\n")
            .arg(port) << flush;

    return app.exec();
}
