# Feature Specification: Dashboard — Horário e Clima de Porto Alegre

**Feature Branch**: `003-lvgl-poa-dashboard`

**Created**: 2026-06-18

**Status**: Draft

**Input**: User description: "Implementar relógio e clima de Porto Alegre usando interface gráfica no painel 128×64, sobre a base de display já validada no milestone DIPONTO."

**Builds on**: `002-diponto-text` (hardware HUB75 128×64 validado). Substitui o splash estático DIPONTO como experiência principal após aprovação e implementação desta feature.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ver horário correto no painel (Priority: P1)

Como operador do painel LED, quero ver a hora atual de forma legível no display 128×64 para saber o horário local de Porto Alegre sem consultar outro dispositivo.

**Why this priority**: O relógio é o caso de uso mais imediato e útil assim que a conectividade e a sincronização de tempo funcionam; entrega valor mesmo antes do clima estar disponível.

**Independent Test**: Conectar o dispositivo a uma rede Wi-Fi estável, aguardar a sincronização de tempo e confirmar que a tela de horário exibe hora coerente com o horário local de Porto Alegre (±1 minuto).

**Acceptance Scenarios**:

1. **Given** o dispositivo ligado com Wi-Fi conectado e tempo ainda não sincronizado, **When** a sincronização de tempo conclui com sucesso, **Then** a tela de horário passa a exibir hora local legível no painel 128×64.
2. **Given** o dispositivo com tempo já sincronizado, **When** passa um intervalo de observação de pelo menos 5 minutos, **Then** a hora exibida permanece coerente com o horário local (sem deriva perceptível além da tolerância de ±1 minuto).
3. **Given** Wi-Fi indisponível após boot, **When** o operador observa a tela de horário, **Then** o painel indica claramente que o horário não está disponível ou não foi sincronizado (estado de erro legível, não tela em branco ou conteúdo ilegível).

---

### User Story 2 - Ver clima de Porto Alegre (Priority: P2)

Como operador do painel LED, quero ver informações de clima atuais para **Porto Alegre**, para acompanhar temperatura e condição do tempo sem abrir outro app.

**Why this priority**: Complementa o relógio com informação útil do dia a dia; depende de conectividade, mas é o segundo marco de produto desta feature.

**Independent Test**: Conectar Wi-Fi e confirmar que a tela de clima mostra dados atuais coerentes com a região metropolitana de Porto Alegre (temperatura e condição resumida legíveis).

**Acceptance Scenarios**:

1. **Given** Wi-Fi conectado e serviço de clima acessível, **When** a primeira consulta de clima conclui com sucesso, **Then** a tela de clima exibe temperatura atual e condição resumida legíveis em 128×64, identificando Porto Alegre como localidade.
2. **Given** clima já obtido com sucesso, **When** a conectividade cai temporariamente, **Then** a tela continua exibindo o último clima conhecido com indicação de que os dados podem estar desatualizados.
3. **Given** serviço de clima indisponível e nenhum dado anterior em cache, **When** o operador observa a tela de clima, **Then** a tela mostra mensagem clara de indisponibilidade, sem travar o restante do dashboard.

---

### User Story 3 - Alternar entre horário e clima (Priority: P3)

Como operador do painel LED, quero alternar automaticamente entre a tela de horário e a de clima para ver ambas as informações no mesmo hardware sem interação física.

**Why this priority**: Melhora a utilidade do único painel físico; não bloqueia entrega das telas individuais.

**Independent Test**: Com ambas as telas funcionais, observar o painel por dois ciclos completos e confirmar transição previsível entre horário e clima.

**Acceptance Scenarios**:

1. **Given** horário sincronizado e clima disponível (ou último cache válido), **When** o dispositivo opera em modo normal, **Then** alterna entre tela de horário e tela de clima em intervalo fixo (padrão: 10 segundos por tela).
2. **Given** apenas a tela de horário está operacional (clima indisponível), **When** o ciclo de alternância ocorre, **Then** a tela de clima exibe seu estado de erro ou indisponibilidade em vez de ocultar a alternância.
3. **Given** boot recente, **When** subsistemas ainda inicializam, **Then** o painel mostra estado de inicialização legível até a primeira tela estar pronta.

---

### Edge Cases

- Wi-Fi conecta após boot tardio: horário e clima devem sincronizar/atualizar sem reinício manual.
- Sincronização de tempo indisponível ou resposta inválida: tela de horário em estado de erro; alternância continua; diagnósticos registram causa acionável.
- Serviço de clima indisponível, timeout ou resposta inválida: exibir último cache se existir; caso contrário, estado de erro na tela de clima.
- Reinício do dispositivo: último clima cacheado (se política de cache aprovada no plano) pode ser reutilizado até nova consulta bem-sucedida.
- Painel desconectado ou falha de driver de display: diagnóstico registrado; dispositivo não entra em loop de reinício por falha de display.
- Memória ou renderização insuficiente para a interface gráfica: falha detectada na inicialização com diagnóstico acionável; contratos de hardware validados no milestone anterior permanecem inalterados.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: O sistema MUST renderizar interface gráfica legível em um painel LED de **128×64 pixels**, preservando o contrato de hardware validado no milestone DIPONTO (preset da placa, sem mapa de GPIO customizado no repositório).
- **FR-002**: O sistema MUST exibir uma **tela de horário** com hora local no formato legível para o operador em 128×64.
- **FR-003**: O sistema MUST obter hora confiável via **sincronização de tempo em rede** quando Wi-Fi estiver disponível.
- **FR-004**: O sistema MUST usar **fuso horário fixo de Porto Alegre** (equivalente a `America/Sao_Paulo`) para exibição do horário local.
- **FR-005**: O sistema MUST exibir uma **tela de clima** com temperatura atual e condição resumida legíveis em 128×64 para **Porto Alegre** (localidade fixa, não configurável pelo operador neste marco).
- **FR-006**: O sistema MUST obter dados de clima de um **serviço público gratuito de previsão meteorológica** para as coordenadas geográficas de Porto Alegre (detalhes do serviço e coordenadas definidos no plano).
- **FR-007**: O sistema MUST conectar-se a **Wi-Fi** como pré-requisito mínimo para sincronização de tempo e consulta de clima.
- **FR-008**: O sistema MUST **alternar automaticamente** entre tela de horário e tela de clima em intervalo fixo (padrão **10 segundos** por tela).
- **FR-009**: O sistema MUST apresentar **estados de erro legíveis** na própria tela quando horário, clima ou conectividade não estiverem disponíveis (sem tela em branco ou conteúdo ilegível).
- **FR-010**: O sistema MUST registrar **diagnósticos acionáveis** em log para falhas de display, interface gráfica, Wi-Fi, sincronização de tempo, consulta de clima e cache, sem expor credenciais ou segredos.
- **FR-011**: O sistema MUST atualizar clima periodicamente quando online (intervalo exato definido no plano; padrão assumido: **30 minutos**) e reutilizar último dado válido quando offline.
- **FR-012**: O sistema MUST limitar o escopo desta feature às telas de **horário** e **clima de Porto Alegre**; MQTT, OTA, provisionamento Wi-Fi por app, configuração de cidade pelo operador e pinagem customizada ficam **fora de escopo** salvo menção explícita em spec/plano posterior.
- **FR-013**: O sistema MUST **não alterar** os contratos de hardware validados no milestone DIPONTO (driver de shift, dimensões 128×64, preset de placa) sem emenda explícita de spec/plano.

### Key Entities

- **Clock state**: Status de sincronização (pendente, sincronizado, erro), instante local exibido, última falha de sincronização diagnosticável.
- **Weather snapshot**: Temperatura atual, condição resumida (texto curto), timestamp da leitura, origem (ao vivo vs cache), localidade fixa Porto Alegre.
- **Dashboard view**: Tela ativa (horário ou clima), estado de erro por tela, indicador de dados desatualizados quando aplicável.
- **Connectivity state**: Wi-Fi conectado/desconectado, impacto sobre sincronização de tempo e clima.
- **Weather cache** (opcional, definido no plano): Último snapshot válido persistido localmente para exibição offline; não inclui configuração de cidade.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Em hardware com painel 128×64 e Wi-Fi estável, **95%** dos boots completam exibição de hora legível em até **2 minutos** após energização.
- **SC-002**: Com Wi-Fi estável, **90%** dos boots exibem clima de Porto Alegre (dados ao vivo ou cache válido) em até **3 minutos** após energização.
- **SC-003**: Operador consegue **ler hora e temperatura a 1 metro de distância** em condições normais de iluminação indoor (validação subjetiva em hardware real documentada nas tarefas).
- **SC-004**: Após perda de Wi-Fi de até **30 minutos**, o painel continua exibindo último clima conhecido ou estado de indisponibilidade claro, **sem reinício espontâneo** atribuível à feature.
- **SC-005**: Falhas de sincronização de tempo, clima ou conectividade produzem mensagem ou indicador de estado reconhecível na tela em **100%** dos cenários de teste definidos nas tarefas (sem tela em branco).
- **SC-006**: Alternância entre horário e clima ocorre de forma previsível: em observação contínua de **5 minutos**, pelo menos **4 ciclos completos** horário↔clima quando ambos subsistemas estão operacionais.

## Assumptions

- Hardware alvo: placa e painel **128×64** validados no milestone `002-diponto-text`; contratos de driver, dimensões e preset de placa permanecem estáveis.
- **Porto Alegre** é localidade **fixa** (não há configuração de cidade pelo operador neste marco); coordenadas geográficas e identificação na tela definidas no plano.
- **Fuso horário** fixo: equivalente a `America/Sao_Paulo`.
- Interface renderizada por **framework gráfico embarcado** sobre o driver de display; detalhes de fontes, cores e layout ficam para `/speckit.plan`.
- **Intervalo de alternância** padrão entre telas: 10 segundos; **atualização de clima**: a cada 30 minutos quando online.
- Serviço de clima é público e gratuito; não requer chave de API neste marco.
- **Idioma** das condições climáticas na tela: português (texto curto) quando suportado pelo serviço ou mapeamento local definido no plano.
- Credenciais Wi-Fi provisionadas por mecanismo definido no plano (fora do escopo UX desta spec); a feature assume conectividade configurável existente ou paralela no plano.
- **Constituição v2.0** será emendada na fase de plano (v2.1) para reativar interface gráfica, Wi-Fi e conectividade como milestone ativo; esta spec depende dessa emenda de governança antes da implementação.
- Sem MQTT, OTA, áudio ou telas adicionais neste marco.
- Feature histórica `001-lvgl-clock-weather` permanece superseded; esta spec substitui o escopo de produto anterior com cidade fixa em Porto Alegre.
