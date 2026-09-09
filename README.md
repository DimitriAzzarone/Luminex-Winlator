# Luminex Windows 0.3

Browser Windows autonomo con motore Chromium incorporato. Non richiede Chrome installato.

## Funzioni

- barra indirizzi, navigazione e schede multiple;
- profili di navigazione persistenti e separati;
- apertura simultanea di profili diversi per usare account Gmail differenti;
- sessioni incognito conservate soltanto in memoria;
- motore Chromium incluso nell'eseguibile;
- nessuna password letta o salvata da Luminex.

Questa versione conserva tutte le funzioni del browser e modifica soltanto la compatibilità: Electron 28, rendering software, sandbox disattivata e pacchetto già estratto.

GitHub Actions produce `Luminex-Windows-0.3-Winlator8.zip`. Estrai l'intero ZIP e avvia `Luminex-Windows.exe`; non spostare il solo eseguibile fuori dalla cartella.

La sandbox è disattivata esclusivamente per compatibilità con Wine/Winlator. La compatibilità con Gmail deve essere verificata inizialmente con un account di prova.
