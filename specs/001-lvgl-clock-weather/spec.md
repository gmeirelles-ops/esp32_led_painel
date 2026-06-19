> **Status: SUPERSEDED** by `002-diponto-text` (2026-06-19). Historical reference only.

# Feature Specification: Dashboard LVGL — Horário e Clima

**Feature Branch**: `001-lvgl-clock-weather`

**Created**: 2026-06-18

**Status**: Draft

**Input**: User description: "Dashboard LVGL em 1 painel 128×64 no MatrixPortal S3: tela de horário (SNTP) e tela de clima (Open-Meteo, cidade configurável via NVS). Usar preset HUB75 da placa, sem pinagem customizada."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ver horário correto no painel (Priority: P1)

Como operador do painel LED, quero ver a hora atual de forma legível no display 128×64 para saber o horário local sem consultar outro dispositivo.

**Why this priority**: O relógio é o caso de uso mais imediato e útil mesmo sem conectividade de clima; entrega valor assim que Wi-Fi e sincronização de tempo funcionam.

**Independent Test**: Provisionar Wi-Fi e fuso horário, aguardar sincronização de tempo e confirmar que a tela de horário exibe hora coerente com a hora local de referência (±1 minuto).

**Acceptance Scenarios**:

1. **Given** o dispositivo ligado com Wi-Fi conectado e tempo ainda não sincronizado, **When** a sincronização de tempo conclui com sucesso, **Then** a tela de horário passa a exibir hora local legível no painel 128×64.
2. **Given** o dispositivo com tempo já sincronizado, **When** passa um intervalo de observação de pelo menos 5 minutos, **Then** a hora exibida permanece coerente com a hora local (sem deriva perceptível além da tolerância de ±1 minuto).
3. **Given** Wi-Fi indisponível após boot, **When** o operador observa a tela de horário, **Then** o painel indica claramente que o horário não está disponível ou não foi sincronizado (estado de erro legível, não tela em branco ou lixo visual).

---

### User Story 2 - Ver clima da cidade configurada (Priority: P2)

Como operador do painel LED, quero ver informações de clima atuais para a cidade que configurei, para acompanhar temperatura e condição sem abrir outro app.

**Why this priority**: Complementa o relógio com informação útil do dia a dia; depende de conectividade e configuração de cidade, mas é o segundo marco de produto definido na constituição.

**Independent Test**: Gravar uma cidade válida na configuração persistente, conectar Wi-Fi e confirmar que a tela de clima mostra dados atuais coerentes com a região configurada.

**Acceptance Scenarios**:

1. **Given** uma cidade válida persistida e Wi-Fi conectado, **When** a primeira consulta de clima conclui com sucesso, **Then** a tela de clima exibe temperatura atual e condição resumida legíveis em 128×64.
2. **Given** clima já obtido com sucesso, **When** a conectividade cai temporariamente, **Then** a tela continua exibindo o último clima conhecido com indicação de que os dados podem estar desatualizados.
3. **Given** cidade inválida ou ausente na configuração persistente, **When** o dispositivo tenta exibir clima, **Then** a tela de clima mostra mensagem clara de configuração pendente ou inválida, sem travar o restante do dashboard.

---

### User Story 3 - Alternar entre horário e clima (Priority: P3)

Como operador do painel LED, quero alternar automaticamente entre a tela de horário e a de clima para ver ambas as informações no mesmo hardware sem interação física.

**Why this priority**: Melhora a utilidade do único painel físico; não bloqueia entrega das telas individuais.

**Independent Test**: Com ambas as telas funcionais, observar o painel por dois ciclos completos e confirmar transição previsível entre horário e clima.

**Acceptance Scenarios**:

1. **Given** horário sincronizado e clima disponível (ou último cache válido), **When** o dispositivo opera em modo normal, **Then** alterna entre tela de horário e tela de clima em intervalo fixo configurável (padrão: 10 segundos por tela).
2. **Given** apenas a tela de horário está operacional (clima indisponível), **When** o ciclo de alternância ocorre, **Then** a tela de clima exibe seu estado de erro ou indisponibilidade em vez de ocultar a alternância.
3. **Given** boot recente, **When** subsistemas ainda inicializam, **Then** o painel mostra estado de inicialização legível até a primeira tela estar pronta.

---

### Edge Cases

- Wi-Fi conecta após boot tardio: horário e clima devem sincronizar/atualizar sem reinício manual.
- SNTP indisponível ou resposta inválida: tela de horário em estado de erro; alternância continua; logs diagnósticos registram causa.
- Open-Meteo indisponível, timeout ou resposta inválida: exibir último cache se existir; caso contrário, estado de erro na tela de clima.
- Cidade configurada inexistente ou fora de cobertura: mensagem clara na tela de clima; não corromper configuração persistente.
- Reinício do dispositivo: cidade e último clima cacheado (se política de cache aprovada no plano) sobrevivem quando aplicável.
- Painel HUB75 desconectado ou falha de driver: diagnóstico registrado; dispositivo não entra em loop de reinício por falha de display.
- Memória ou renderização insuficiente para LVGL: falha detectada na inicialização com log acionável (sem pinagem customizada — preset da placa permanece contrato).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: O sistema MUST renderizar interface gráfica legível em um painel LED HUB75 de **128×64 pixels** usando o preset de hardware da Adafruit MatrixPortal S3, **sem mapa de GPIO customizado** no repositório.
- **FR-002**: O sistema MUST exibir uma **tela de horário** com hora local no formato legível para o operador em 128×64.
- **FR-003**: O sistema MUST obter hora confiável via **sincronização de tempo em rede (SNTP)** quando Wi-Fi estiver disponível.
- **FR-004**: O sistema MUST respeitar **fuso horário** configurável de forma persistente, com fallback documentado no plano quando ausente.
- **FR-005**: O sistema MUST exibir uma **tela de clima** com temperatura atual e condição resumida legíveis em 128×64.
- **FR-006**: O sistema MUST obter dados de clima de **Open-Meteo** para a **cidade configurada via armazenamento persistente (NVS)**.
- **FR-007**: O sistema MUST persistir a **cidade** (e metadados mínimos necessários à consulta, ex.: coordenadas ou identificador geográfico) de forma compatível com dispositivos já provisionados; alterações de schema exigem migração explícita no plano.
- **FR-008**: O sistema MUST conectar-se a **Wi-Fi** como pré-requisito mínimo para SNTP e consulta de clima.
- **FR-009**: O sistema MUST **alternar automaticamente** entre tela de horário e tela de clima em intervalo fixo (padrão **10 segundos** por tela), configurável no plano sem quebrar compatibilidade de NVS já aprovada.
- **FR-010**: O sistema MUST apresentar **estados de erro legíveis** na própria tela quando horário, clima ou conectividade não estiverem disponíveis (sem tela em branco ou conteúdo ilegível).
- **FR-011**: O sistema MUST registrar **diagnósticos acionáveis** em log para falhas de display, LVGL, Wi-Fi, SNTP, HTTP/clima e NVS, sem expor credenciais ou segredos.
- **FR-012**: O sistema MUST atualizar clima periodicamente quando online (intervalo exato definido no plano; padrão assumido: **30 minutos**) e reutilizar último dado válido quando offline.
- **FR-013**: O sistema MUST limitar o escopo desta feature às telas de **horário** e **clima**; MQTT, OTA, provisionamento Wi-Fi por app e pinagem customizada ficam **fora de escopo** salvo menção explícita em spec/plano posterior.

### Key Entities

- **Device configuration (NVS)**: Cidade para clima, fuso horário e parâmetros mínimos de consulta geográfica; sobrevive a reinícios; sujeito a contrato de compatibilidade.
- **Clock state**: Status de sincronização (pendente, sincronizado, erro), instante local exibido, última falha SNTP diagnosticável.
- **Weather snapshot**: Temperatura atual, condição resumida (texto curto), timestamp da leitura, origem (ao vivo vs cache).
- **Dashboard view**: Tela ativa (horário ou clima), estado de erro por tela, indicador de dados desatualizados quando aplicável.
- **Connectivity state**: Wi-Fi conectado/desconectado, impacto sobre SNTP e clima.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Em hardware MatrixPortal S3 com painel 128×64 e Wi-Fi estável, **95%** dos boots completam exibição de hora legível em até **2 minutos** após energização.
- **SC-002**: Com cidade válida configurada e Wi-Fi estável, **90%** dos boots exibem clima atual ou cache válido em até **3 minutos** após energização.
- **SC-003**: Operador consegue **ler hora e temperatura a 1 metro de distância** em condições normais de iluminação indoor (validação subjetiva em hardware real documentada nas tarefas).
- **SC-004**: Após perda de Wi-Fi de até **30 minutos**, o painel continua exibindo último clima conhecido ou estado de indisponibilidade claro, **sem reinício espontâneo** atribuível à feature.
- **SC-005**: Falhas de SNTP, clima ou NVS produzem mensagem ou ícone de estado reconhecível na tela em **100%** dos cenários de teste definidos nas tarefas (sem tela em branco).
- **SC-006**: Alternância entre horário e clima ocorre de forma previsível: em observação contínua de **5 minutos**, pelo menos **4 ciclos completos** horário↔clima quando ambos subsistemas estão operacionais.

## Assumptions

- Hardware alvo: **Adafruit MatrixPortal S3** com chicote HUB75 de fábrica e **um módulo 128×64**; driver e pinagem seguem preset **esphome/esp-hub75** (MatrixPortal), conforme constituição — não há pinagem customizada neste repositório.
- UI renderizada com **LVGL** sobre o driver HUB75; detalhes de fontes, cores e layout exato ficam para `/speckit.plan`.
- **Provisionamento da cidade** na NVS fora do fluxo UX desta feature (ferramenta de desenvolvimento, comando serial ou processo definido no plano); não inclui app Flutter nem BLE/Blufi neste marco.
- **Fuso horário** padrão assumido: `America/Sao_Paulo` quando não configurado, até definição contrária no plano.
- **Intervalo de alternância** padrão entre telas: 10 segundos; **atualização de clima**: a cada 30 minutos quando online.
- Open-Meteo é serviço público gratuito; não requer chave de API neste marco.
- **Idioma** das condições climáticas na tela: português (texto curto) quando suportado pela API ou mapeamento local definido no plano.
- Wi-Fi já provisionado ou mecanismo mínimo de credenciais definido fora desta spec (template diponto); esta feature assume conectividade configurável existente ou paralela no plano.
- Sem MQTT, OTA, áudio (ESP-ADF) ou telas adicionais neste marco.
