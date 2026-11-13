# Fork FabioRM/Frekvens: piano di lavoro

Fork di [VIPnytt/Frekvens](https://github.com/VIPnytt/Frekvens) basato sul rilascio **v2.5.2**.
Obiettivo: una gestione web molto più curata (ispirata al progetto `rgb_matrix_12x12`),
supporto multifont completo e le funzioni del rgb-clock, mantenendo tutto ciò che Frekvens
già offre (33 modalità, Alexa, Home Assistant/MQTT, REST, OTA, playlist, ecc.).

## Perché v2.5.2 e non upstream `main`

`main` upstream è la 3.0.0-alpha: ha rimosso OTA sulle schede da 4MB, le estensioni Heap,
Server-Sent Events e Status LED, la modalità Binary Clock, il font Braille, l'hotspot Wi-Fi,
l'upload firmware dalla web app e i provider meteo con API key. Le correzioni upstream utili
verranno portate con cherry-pick.

## Configurazione locale (fatto)

| File | Tracciato | Contenuto |
|---|---|---|
| `.env` | sì | valori predefiniti del fork (funzioni senza hardware dedicato) |
| `.env.<dispositivo>` | no | dispositivo, `NAME`, `HOSTNAME`, `OTA_KEY`, override (`'false'` per spegnere) |
| `firmware/include/config/secrets.h` | sì | modello con esempi |
| `firmware/include/config/secrets_local.h` | no | pin, Wi-Fi, posizione |

Ogni env PlatformIO sceglie il proprio file con `custom_dotenv`:

| Env | Dispositivo | Upload |
|---|---|---|
| `frekvens` | IKEA Frekvens, ESP32-C6-DevKitC-1 8MB | USB |
| `frekvens_ota` | come sopra | Wi-Fi (`frekvens.local`) |
| `obegransad` | IKEA Obegränsad, Waveshare ESP32-S3-Zero 4MB | USB |

## Fase 1: multifont (firmware)

1. **Font misti nello stesso testo.** Markup `{font:Nome}` dentro messaggi e ticker per
   cambiare font a metà testo; `{font:}` torna al font predefinito. Il rendering
   (`TextHandler`) gestisce segmenti con font diversi allineati alla stessa linea di base.
2. **Scelta del font per ogni testo.** Tutte le modalità che scrivono testo (orologio,
   countdown, messaggi, ticker, meteo, timer) espongono la scelta del font via WebSocket/REST.
3. **Nuovi font.** Convertitore `tools/gfx2frekvens.py` dal formato Adafruit GFX al formato
   `FontModule` di Frekvens. Da includere: Roboto Mono Bold 6 (Apache 2.0).
   I font di Rob Jennings (2x5, 3x5, 3x7, 4x5, 4x7, 5x5, 5x7 e varianti mono) **non hanno una
   licenza**: non possono stare nel fork pubblico finché l'autore non ne concede l'uso.
   Alternative: generarli solo in locale (git-ignored) con il convertitore, oppure disegnare
   font originali con le stesse dimensioni.

## Fase 2: funzioni del rgb-clock (firmware)

1. **Luminosità notturna**: livello separato e attivazione da orario fisso o dal tramonto
   all'alba (posizione da `LATITUDE`/`LONGITUDE`).
2. **Modalità Auto**: orologio con schermate informative a rotazione (meteo, countdown,
   animazione) a intervalli configurabili, costruita sopra la playlist esistente.
3. **Timer / Pomodoro**: nuova modalità con lavoro/pausa, avvio/pausa/reset da web, REST e
   pulsanti.
4. **Animazioni da GIF**: la web app converte GIF/PNG/JPG/WebP in frame 16x16 a toni di grigio
   e li salva sul dispositivo (LittleFS) per la modalità Animation; libreria con anteprime,
   riproduzione ed eliminazione.

## Fase 3: nuova web app

Riscrittura di `webapp/src` (SolidJS + Vite + Tailwind, file singolo in LittleFS) con il
layout del rgb-clock:

- sidebar fissa su desktop, drawer su mobile, tema chiaro/scuro, accento arancione;
- **Home**: anteprima live della matrice (estensione Screenshot), ora, griglia delle modalità
  con icone, impostazioni rapide (accensione, luminosità, messaggio);
- **Display**: luminosità giorno/notte, orientamento, programmazione notturna;
- **Modalità**: impostazioni di ogni modalità in card, con font scelto da anteprime reali;
- **Font**: catalogo con anteprima dei glifi e test di testo con markup misto;
- **Playlist / Auto**, **Timer**, **Countdown**, **Meteo**, **Disegno**, **Animazioni**;
- **Integrazioni**: Alexa, Home Assistant, MQTT, REST con esempi `curl` pronti;
- **Sistema**: dispositivo, Wi-Fi, memoria (Heap), OTA, riavvio.

Il protocollo WebSocket esistente (messaggi JSON indicizzati per nome modulo) resta invariato,
così REST, MQTT e Home Assistant continuano a funzionare.

## Verifica

Ogni fase: build degli env `frekvens` e `obegransad`, aggiornamento OTA del Frekvens,
controllo via REST/WebSocket e nel browser.
