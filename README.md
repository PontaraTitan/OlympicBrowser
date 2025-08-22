# OlympicBrowser
Exploração interativa para análise de dados de 120 anos de história olímpica baseada na metodologia CRISP-DM e no Framework Qt
Uma aplicação desktop moderna desenvolvida em C++ com Qt para visualização e análise interativa de dados históricos dos Jogos Olímpicos (1896-2016). O projeto processa mais de 270.000 registros de atletas olímpicos, oferecendo insights sobre tendências esportivas ao longo de mais de um século.
🎯 Características Principais

📊 Visualização Dual: Interface tabular com filtros avançados e interface gráfica com 5 modalidades analíticas
⚡ Alta Performance: Processamento de 270k+ registros em menos de 1 segundos
🎨 Interface Moderna: Design intuitivo com componentes Qt Widgets nativos
📈 Análises Avançadas: Gráficos interativos com Qt Charts para análise temporal, demográfica e geográfica
📤 Exportação Múltipla: Suporte a CSV, Excel, JSON, HTML e PDF
🔍 Sistema de Cache: Otimização para consultas repetitivas

🏗️ Arquitetura
O projeto implementa o padrão Model-View-Controller (MVC) com arquitetura modular:
📦 OlympicBrowser
├── 🗃️ Model Layer
│   ├── Database (Singleton)
│   ├── OlympicTableModel
│   └── OlympicFilterProxyModel
├── 🎮 Controller Layer
│   └── Controller
├── 🖥️ View Layer
│   ├── MainWindow
│   ├── OlympicTableView
│   └── OlympicGraphView
└── 🛠️ Utilities
    ├── ExportManager
    └── ReportDialog
📊 Modalidades de Visualização
Interface Tabular

Filtragem multicritério em tempo real
Busca textual avançada
Ordenação por qualquer coluna
Exportação de dados filtrados

Interface Gráfica (5 Categorias)

📈 Evolução de Medalhas: Análise temporal do desempenho por país
👥 Distribuição Demográfica: Evolução da participação por gênero
🌍 Comparação entre Países: Análise comparativa de desempenho
🗺️ Visualização Geográfica: Distribuição geográfica de medalhas por edição
📊 Análise Estatística: Correlações entre atributos físicos e performance

🛠️ Tecnologias Utilizadas

Linguagem: C++17
Framework GUI: Qt 6.x (compatível com Qt 5.x)
Build System: CMake 3.5+
Metodologia: CRISP-DM (Cross-Industry Standard Process for Data Mining)
Padrões: MVC, Singleton, Observer (Signals/Slots)

Dependências Qt

Qt Widgets (Interface gráfica)
Qt Charts (Visualizações gráficas)
Qt PrintSupport (Geração de relatórios)
Qt Svg (Suporte a gráficos vetoriais)

🚀 Instalação e Compilação
Pré-requisitos

Qt 6.x ou Qt 5.x
CMake 3.5 ou superior
Compilador C++17 (GCC, Clang, MSVC)

1. Clonar o Repositório
bashgit clone https://github.com/renanpontara/OlympicBrowser.git
cd OlympicBrowser
2. Configurar e Compilar
bashmkdir build
cd build
cmake ..
make
3. Windows (Visual Studio)
cmdmkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
4. Executar
bash# Linux/macOS
./OlympicBrowser

# Windows
OlympicBrowser.exe
📁 Estrutura de Dados
O projeto utiliza o dataset "120 Years of Olympic History" disponível no Kaggle:

271.116 registros de atletas olímpicos
15 atributos por registro
Período: 1896-2016 (Jogos de Verão e Inverno)
Formato: CSV com tratamento de valores ausentes

💻 Como Usar
1. Carregamento de Dados
A aplicação carrega automaticamente o arquivo datasets/athlete_events.csv na inicialização.

2. Modo Tabular
Use os controles de filtro para refinar os dados
Clique nos cabeçalhos para ordenar colunas
Utilize a busca textual para encontrar atletas específicos
Exporte resultados usando o botão "Exportar"

3. Modo Gráfico
Selecione a modalidade desejada no menu superior
Configure os parâmetros específicos de cada análise
Interaja com os gráficos para obter informações detalhadas
Gere relatórios personalizados com gráficos e dados

📈 Performance
Carregamento: < 1 segundos para 270k registros
Filtragem: Tempo real com sistema de cache
Memória: ~150MB para dataset completo
Responsividade: Interface assíncrona sem travamentos

📄 Licença
Este projeto está licenciado sob a MIT License - veja o arquivo LICENSE para detalhes.
👨‍💻 Autor
Renan Cezar Girardin Pimentel Pontara

🎓 Contexto Acadêmico
Este projeto foi desenvolvido como Trabalho de Conclusão de Curso (TCC) para o MBA em Engenharia de Software da USP/Esalq, aplicando a metodologia CRISP-DM para análise de dados históricos esportivos.
🙏 Agradecimentos

Orientadora: Prof.ª Daniele Aparecida Cicillini Pimenta (PECEGE/USP)
Dataset: "120 Years of Olympic History" disponível no Kaggle
Framework: Qt Project pela excelente documentação e ferramentas

📚 Referências

Chapman, P. et al. (2000). CRISP-DM 1.0: Step-by-step data mining guide
Blanchette, J., & Summerfield, M. (2006). C++ GUI Programming with Qt 4
Kaggle Dataset: 120 Years of Olympic History
