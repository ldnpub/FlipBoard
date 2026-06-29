# FlipBoard CONCOURSE — Pebble Store listing

## Tagline
A bilingual airport departure board for your wrist — in crisp 14-segment.

## Description (English — store default)
FlipBoard CONCOURSE turns your Pebble Time 2 into a full airport-concourse
departure board. A "PEBBLE-DEPARTS" header carries the time, and below it your day
is laid out as a proper information board — row after row of crisp 14-segment
alphanumerics, white with grey anti-aliased edges, the big-airport split-flap
energy you only get standing under a real terminal display. On every minute the
tiles fold over with a clean split-flap snap.

What makes CONCOURSE special: it's a truly bilingual board. Every row carries BOTH
its French and English label stacked together — PAS/STEPS, HEURE/TIME — so the
board reads in two languages at once.

Underneath the header, the board lays out your day:

• STEPS, colour-coded against your goal, with a percent-to-goal. The number starts
  warm red when you're near zero, warms through amber and yellow across the day, and
  turns green as you close in on your target — so a single look tells you how you're
  tracking.
• TIME, with an "ON TIME" / "A L HEURE" status flag — departures-board style.
• DATE, with the weekday, in clean 14-segment type.
• BATTERY, with an "OK" / "LOW" status flag.

Make it yours from the Pebble phone app:

• Daily step goal — the colour ramp follows whatever target you set.
• Language — CONCOURSE speaks Français, English, Deutsch, Español, Italiano,
  Nederlands, Português, Polski and Svenska. Labels and weekdays follow along.
• Date format — Auto (it matches your language: DD-MM, MM-DD or ISO), or pick the
  order by hand. On this segment board the separator is always a dash, the way real
  concourse displays show it.
• Year — hide it, or show it as "26" or "2026" when there's room.
• Wrist shake — off by default, or assign one action: replay the flip animation,
  trigger the backlight, flash your steps-to-goal, surface a random destination
  code (a little easter egg), or toggle 12h/24h.
• 12h / 24h — follows your watch by default; the shake toggle can flip it on the fly.

Designed for the Pebble Time 2 (Emery, 200×228) colour display, with anti-aliased
custom digits and an e-paper-calibrated palette so it looks crisp on the real
panel — not just in the emulator.

## Description (Français)
FlipBoard CONCOURSE transforme votre Pebble Time 2 en véritable tableau de départs
de hall d'aéroport. Un en-tête « PEBBLE-DEPARTS » porte l'heure, et en dessous votre
journée s'affiche comme un vrai tableau d'information — ligne après ligne d'alpha-
numériques à 14 segments, blanc aux bords anti-aliasés gris, l'énergie des grands
afficheurs à volets qu'on ne ressent que sous un vrai tableau de terminal. À chaque
minute les tuiles se replient d'un coup net.

Ce qui rend CONCOURSE spécial : c'est un tableau vraiment bilingue. Chaque ligne
porte À LA FOIS son libellé français et anglais empilés — PAS/STEPS, HEURE/TIME —
si bien que le tableau se lit dans deux langues d'un coup.

Sous l'en-tête, le tableau déroule votre journée :

• Les PAS, dont la couleur suit votre objectif, avec le pourcentage atteint : rouge
  près de zéro, puis ambre et jaune dans la journée, et vert à l'approche du but.
• L'HEURE, avec un indicateur d'état « A L HEURE » / « ON TIME ».
• La DATE, avec le jour de la semaine, en typographie nette à 14 segments.
• La BATTERIE, avec un indicateur « OK » / « LOW ».

Personnalisable depuis l'application téléphone :

• Objectif de pas quotidien — le dégradé de couleur s'y adapte.
• Langue — Français, English, Deutsch, Español, Italiano, Nederlands, Português,
  Polski, Svenska. Libellés et jours de la semaine suivent.
• Format de date — Auto (selon la langue : JJ-MM, MM-JJ ou ISO), ou l'ordre choisi
  à la main. Sur ce tableau à segments le séparateur est toujours un tiret, comme
  sur les vrais afficheurs de hall.
• Année — masquée, ou affichée en « 26 » ou « 2026 » quand il y a la place.
• Secousse du poignet — désactivée par défaut, ou une action au choix : rejouer
  l'animation à volets, allumer le rétroéclairage, afficher les pas restants
  jusqu'à l'objectif, faire surgir un code destination aléatoire (petit easter
  egg), ou basculer 12h/24h.
• 12h / 24h — suit la montre par défaut ; la secousse peut basculer à la volée.

Conçue pour l'écran couleur du Pebble Time 2 (Emery, 200×228), chiffres
anti-aliasés et palette calibrée pour le vrai panneau e-paper.

## Source
Open source — github.com/ldnpub/FlipBoard

## Category
daily

## Release notes (this version)
BETA — new phone settings: date format (Auto-by-language or manual order; dash
separator on the segment display), year display (off · 26 · 2026), and a
configurable wrist-shake action (off by default; replay flip · backlight ·
steps-to-goal · random destination · toggle 12/24h). Language now has its own menu
(no longer tied to shake). Plus the configurable daily step goal and 9-language
labels (FR/EN/DE/ES/IT/NL/PT/PL/SV).
