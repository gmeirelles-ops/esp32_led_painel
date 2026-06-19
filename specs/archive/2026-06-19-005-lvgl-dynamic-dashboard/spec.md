# Feature Specification: Dashboard LVGL Dinâmico

**Feature Branch**: `005-lvgl-dynamic-dashboard`

**Created**: 2026-06-19

**Status**: Draft

**Input**: User description: "vamos usufruir do lvgl e deixa mais dinamica essas telas e layout, esta aparecen sem hora agora tambem"

**Builds on**: `003-lvgl-poa-dashboard` (horário + clima Porto Alegre), `004-hub75-2x2-topology` (canvas 128×64 ou 256×128 configurável em build).

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ver a hora de forma confiável (Priority: P1)

Como operador do painel, quero que a tela de horário **sempre mostre a hora local** quando o relógio estiver sincronizado, para não ficar com tela vazia ou mensagem enganosa enquanto o horário já está disponível.

**Why this priority**: Regressão reportada — a hora não aparece hoje; sem isso o dashboard perde a função principal.

**Independent Test**: Com Wi-Fi estável, aguardar sincronização de tempo e confirmar que hora e data aparecem de forma contínua por pelo menos 5 minutos, atualizando a cada minuto.

**Acceptance Scenarios**:

1. **Given** Wi-Fi conectado e tempo sincronizado com sucesso, **When** a tela de horário está ativa, **Then** hora no formato `HH:MM` e data resumida são exibidas de forma legível (não vazias).
2. **Given** tempo sincronizado, **When** a tela alterna para clima e volta ao horário, **Then** a hora reaparece imediatamente sem ficar em branco.
3. **Given** Wi-Fi indisponível ou tempo ainda não sincronizado, **When** a tela de horário está ativa, **Then** uma mensagem clara de indisponibilidade é exibida (nunca hora vazia sem explicação).
4. **Given** horário de Porto Alegre (UTC−3), **When** comparado com referência confiável, **Then** desvio máximo de ±1 minuto após sincronização.

---

### User Story 2 - Layout que se adapta ao tamanho do painel (Priority: P1)

Como operador com painel único (128×64) ou grade 2×2 (256×128), quero que horário e clima **preencham bem o espaço disponível**, com tipografia e espaçamento proporcionais, para leitura confortável à distância.

**Why this priority**: Com topologia 2×2 o conteúdo atual fica pequeno e centralizado; aproveitar melhor o canvas é o ganho principal desta feature.

**Independent Test**: Validar legibilidade em ambos os perfis de display (single e 2×2) sem alterar o conteúdo informativo (hora, data, cidade, temperatura, condição).

**Acceptance Scenarios**:

1. **Given** painel 128×64, **When** qualquer tela do dashboard está ativa, **Then** elementos principais ocupam a área útil com margens equilibradas e texto legível a ~1 m.
2. **Given** painel 256×128 (2×2), **When** qualquer tela está ativa, **Then** tipografia e espaçamento escalam para usar a resolução maior (não permanecem “minúsculos” no centro).
3. **Given** mudança de perfil de build (single vs 2×2), **When** o firmware inicia, **Then** o layout correto é aplicado automaticamente sem configuração manual pelo operador.

---

### User Story 3 - Transições e feedback visual entre telas (Priority: P2)

Como operador, quero **transições suaves** entre horário e clima e indicação visual de carregamento ou erro, para perceber que o painel está vivo e entender o estado sem ler logs.

**Why this priority**: Eleva a experiência usando recursos nativos de interface dinâmica; não bloqueia correção do relógio nem layout responsivo.

**Independent Test**: Observar pelo menos 4 ciclos completos horário↔clima e estados de boot, Wi-Fi pendente e erro de clima.

**Acceptance Scenarios**:

1. **Given** ambas as telas operacionais, **When** ocorre alternância automática (10 s), **Then** a mudança usa transição visual perceptível (ex.: fade ou slide) sem flicker ou tela preta prolongada (>500 ms).
2. **Given** boot em andamento, **When** subsistemas ainda não prontos, **Then** estado “Iniciando…” ou equivalente permanece visível até a primeira tela útil.
3. **Given** clima em atualização ou indisponível, **When** a tela de clima está ativa, **Then** o operador distingue dados ao vivo, cache desatualizado e erro total por ícones, cores ou rótulos curtos.

---

### Edge Cases

- SNTP sincroniza após a tela de horário já ter mostrado “Sem hora”: a hora deve aparecer automaticamente em até 2 s sem reinício.
- Relógio cruza meia-noite ou muda o dia: data e hora atualizam corretamente.
- Alternância durante animação de transição: não corrompe layout nem deixa objetos sobrepostos ilegíveis.
- Memória limitada em 256×128: animações leves não devem impedir atualização periódica do relógio.
- Wi-Fi oscila (conecta/desconecta): estados de erro e recuperação são claros e o horário retorna quando sincronização volta.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: O sistema MUST exibir hora local (`HH:MM`) e data resumida na tela de horário sempre que o tempo estiver sincronizado; campos de hora/data MUST NOT permanecer vazios nesse estado.
- **FR-002**: O sistema MUST corrigir a regressão em que a hora não aparece após sincronização bem-sucedida (inclui reaplicação de fuso horário de Porto Alegre e atualização imediata da UI ao receber evento de sync).
- **FR-003**: O sistema MUST adaptar layout, tamanho de fonte e espaçamento das telas de horário e clima à resolução lógica do display (128×64 ou 256×128).
- **FR-004**: O sistema MUST manter contraste alto (fundo escuro, texto claro) e legibilidade a ~1 m em ambas as resoluções.
- **FR-005**: O sistema MUST aplicar transição visual entre tela de horário e clima na alternância automática (duração total da transição ≤ 500 ms, configurável no plano).
- **FR-006**: O sistema MUST distinguir visualmente estados: sincronizando, sincronizado, sem Wi-Fi, sem hora, clima ao vivo, clima em cache desatualizado e clima indisponível.
- **FR-007**: O sistema MUST preservar alternância automática a cada 10 s e conteúdo informativo existente (Porto Alegre fixo, temperatura, condição em português).
- **FR-008**: O sistema MUST registrar em log eventos de transição de cena e falhas de exibição de horário, sem credenciais.
- **FR-009**: O sistema MUST NOT alterar contratos de hardware (pinagem, FM6126A, topologia configurável em build) nem adicionar novas telas além de horário e clima neste marco.

### Key Entities

- **Scene layout profile**: Parâmetros visuais derivados da resolução (fontes escalonadas, margens, alinhamentos).
- **Clock presentation**: Hora, data, estado de sync, mensagem de erro; regra de visibilidade (nunca vazio sem motivo).
- **Weather presentation**: Cidade, temperatura, condição, indicador stale/erro; hierarquia visual.
- **Transition state**: Tela origem/destino, animação em curso, bloqueio de input (N/A em painel sem toque).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Em 100% dos testes com Wi-Fi estável e tempo sincronizado, a hora legível aparece em até **2 minutos** após boot e permanece visível durante observação de **10 minutos**.
- **SC-002**: Em hardware 128×64 e 256×128, **90%** dos observadores leem hora e temperatura a **1 metro** sem ambiguidade (validação documentada em campo).
- **SC-003**: Transição horário↔clima completa em **≤ 500 ms** em **95%** dos ciclos observados em teste de **5 minutos**.
- **SC-004**: Em cenários de erro (sem Wi-Fi, sem hora, sem clima), **100%** das exibições mostram mensagem ou ícone reconhecível — nunca tela com campos principais vazios sem legenda.
- **SC-005**: Após sincronização tardia de tempo, a hora correta aparece em até **2 segundos** sem reinício do dispositivo.

## Assumptions

- Porto Alegre e fuso `America/Sao_Paulo` (exibição local UTC−3) permanecem fixos; correção de timezone faz parte do escopo da regressão de horário.
- Apenas telas de **horário** e **clima**; sem novos widgets (gráficos, previsão multi-dia, touch).
- Animações LVGL leves (fade/slide); sem vídeo ou sprites pesados.
- Intervalo de alternância 10 s e atualização de clima 30 min mantidos salvo ajuste documentado no plano.
- Build single (128×64) continua sendo default; layout 2×2 herda mesma lógica de escalonamento.
- Regressão “sem hora” pode estar ligada a fuso horário, estado de sync ou refresh da UI — o plano detalhará causa raiz; a spec exige o resultado correto para o operador.
