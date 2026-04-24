#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>

std::mutex fileMutex;

// Retorna o timestamp atual no formato HH:MM:SS.mmm
std::string timestamp() {
    auto now   = std::chrono::system_clock::now();
    auto ms    = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm    = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

// Subsistemas simulados da nave Artemis III
const std::vector<std::string> subsystems = {
    "PROPULSAO",
    "NAVEGACAO",
    "SUPORTE_DE_VIDA",
    "COMUNICACOES",
    "ENERGIA",
    "ESTRUTURA",
    "COMPUTADOR_CENTRAL",
    "RADAR_LUNAR",
    "MODULO_CIENTIFICO",
    "CONTROLE_TERMICO"
};

// Mensagens de operação por subsistema
const std::vector<std::string> operacoes = {
    "Verificacao de rotina concluida com sucesso.",
    "Leitura de sensores OK — valores dentro do esperado.",
    "Calibracao automatica iniciada.",
    "Sincronizacao com Houston confirmada.",
    "Alerta amarelo — anomalia detectada. Investigando.",
    "Manobra de correcao de trajetoria executada.",
    "Consumo de energia estabilizado.",
    "Pressurização do modulo verificada.",
    "Download de telemetria transmitido para Terra.",
    "Backup de dados criado com sucesso.",
    "Temperatura dos paineis solares: nominal.",
    "Oxigenio disponivel: 98.7% — reserva adequada.",
    "GPS lunar ativo — posicao confirmada.",
    "Protocolo de emergencia testado — sem falhas.",
    "Registro de hora sincronizado com GPS.",
};

// Função executada por cada thread
// id      → número da thread (0–19)
// arquivo → nome do arquivo de log
// nLogs   → quantos registros cada thread deve gerar
void registrarLogs(int id, const std::string& arquivo, int nLogs) {
    // Gerador de números aleatórios local por thread (sem compartilhamento)
    std::mt19937 rng(std::random_device{}() + id);
    std::uniform_int_distribution<int> distSub(0, (int)subsystems.size() - 1);
    std::uniform_int_distribution<int> distOp (0, (int)operacoes.size()  - 1);
    std::uniform_int_distribution<int> distMs (5, 30);  // simula tempo de trabalho

    for (int i = 0; i < nLogs; ++i) {
        // Simula tempo de processamento real do subsistema
        std::this_thread::sleep_for(std::chrono::milliseconds(distMs(rng)));

        // Monta a linha de log ANTES de travar o mutex
        std::ostringstream linha;
        linha << "[" << timestamp() << "]"
              << " [THREAD-" << std::setfill('0') << std::setw(2) << id << "]"
              << " [" << subsystems[distSub(rng)] << "]"
              << " " << operacoes[distOp(rng)]
              << "\n";

        // ── Seção crítica ────────────────────────────────────────────────
        //  lock_guard libera o mutex automaticamente ao sair do escopo
        {
            std::lock_guard<std::mutex> lock(fileMutex);
            std::ofstream ofs(arquivo, std::ios::app);
            if (ofs.is_open()) {
                ofs << linha.str();
            }
        }
        // ── Fim da seção crítica ─────────────────────────────────────────
    }
}

int main() {
    const std::string ARQUIVO = "artemis_III_missao.log";
    const int NUM_THREADS     = 20;
    const int LOGS_POR_THREAD = 10;  // cada thread gera 10 entradas → 200 no total

    // Cabeçalho do arquivo de log
    {
        std::ofstream ofs(ARQUIVO, std::ios::trunc);
        ofs << "╔══════════════════════════════════════════════════════════════╗\n"
            << "║         ARTEMIS III — LOG DE OPERAÇÕES DA MISSÃO             ║\n"
            << "║         " << NUM_THREADS << " subsistemas monitorados em paralelo               ║\n"
            << "╚══════════════════════════════════════════════════════════════╝\n\n";
    }

    std::cout << "Artemis III: iniciando " << NUM_THREADS << " threads de monitoramento...\n";

    // Cria e lança todas as threads
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(registrarLogs, i, ARQUIVO, LOGS_POR_THREAD);
    }

    // Aguarda todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    std::cout << " Missão registrada! " << (NUM_THREADS * LOGS_POR_THREAD)
              << " entradas salvas em \"" << ARQUIVO << "\".\n";

    return 0;
}
