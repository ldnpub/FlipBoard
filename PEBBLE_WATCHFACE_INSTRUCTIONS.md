# Pebble Watchface — Consignes de développement, build et publication

> Référence officielle : https://developer.rebble.io  
> Cible principale : **Pebble Time 2** (plateforme `emery`, 200×228 px, couleur 64 couleurs)  
> Langage : **C** via le Pebble SDK

---

## 1. Structure du projet

Chaque watchface est un projet autonome avec la structure suivante :

```
my-watchface/
├── src/
│   └── c/
│       └── my-watchface.c   # Code principal
├── resources/
│   └── images/              # Assets PNG/bitmap
├── package.json             # Métadonnées et config SDK
├── wscript                  # Build script (généré par SDK)
├── .gitignore
└── README.md
```

### package.json — template cible Time 2

```json
{
  "name": "my-watchface",
  "author": "Ton Nom",
  "version": "1.0.0",
  "keywords": ["pebble-app"],
  "private": false,
  "dependencies": {},
  "pebble": {
    "displayName": "My Watchface",
    "uuid": "<UUID_GÉNÉRÉ>",
    "sdkVersion": "3",
    "enableMultiJS": true,
    "targetPlatforms": ["emery", "basalt", "diorite", "chalk"],
    "watchapp": {
      "watchface": true
    },
    "resources": {
      "media": []
    }
  }
}
```

> **Règle** : le champ `uuid` doit toujours être généré avec `uuidgen` ou `pebble new-project`. Ne jamais le modifier manuellement ni le copier-coller depuis un autre projet.

---

## 2. Bonnes pratiques de code C

### 2.1 Ne jamais hardcoder les dimensions d'écran

```c
// ❌ MAUVAIS — cassé sur les autres plateformes
s_layer = layer_create(GRect(0, 0, 144, 168));

// ✅ BON — s'adapte à toutes les résolutions
Layer *window_layer = window_get_root_layer(window);
GRect bounds = layer_get_unobstructed_bounds(window_layer);
s_layer = layer_create(bounds);
```

### 2.2 Toujours gérer les différences de plateforme avec les macros SDK

```c
// Couleur vs noir & blanc
window_set_background_color(s_window,
  PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));

// Affichage conditionnel selon la plateforme cible
#if defined(PBL_PLATFORM_EMERY)
  // Code spécifique Pebble Time 2 (200x228)
#elif defined(PBL_COLOR)
  // Code pour plateformes couleur génériques
#else
  // Fallback noir & blanc
#endif
```

### 2.3 Économiser la batterie — règles critiques

```c
// ❌ MAUVAIS — réveille le processeur 60 fois par minute
tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

// ✅ BON — sauf si les secondes sont affichées
tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

// ✅ Animations : ne jouer qu'au mouvement du poignet, pas en continu
static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  play_animation();
}
accel_tap_service_subscribe(accel_tap_handler);
```

**Règles batterie à respecter :**
- Utiliser `MINUTE_UNIT` par défaut, `SECOND_UNIT` uniquement si l'affichage des secondes est explicitement demandé
- Grouper les mises à jour d'accéléromètre en batches (`accel_service_set_sampling_rate`)
- Toujours `unsubscribe` les services dans `window_unload`

### 2.4 Architecture modulaire

Séparer le code en modules logiques :

```
src/c/
├── main.c           # init() / deinit() uniquement
├── watchface.c      # logique de la face
├── watchface.h
├── ui_layers.c      # création et layout des layers
├── ui_layers.h
└── data_handler.c   # abonnements tick, météo, health...
```

### 2.5 Gestion mémoire

```c
// Toujours libérer dans l'ordre inverse de création
static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_canvas_layer);
}

static void deinit(void) {
  window_destroy(s_main_window);
}
```

---

## 3. Workflow de build

### Commandes essentielles

```bash
# Créer un nouveau projet (génère UUID, structure, wscript)
pebble new-project --simple my-watchface

# Builder pour toutes les plateformes
pebble build

# Tester dans l'émulateur QEMU (Time 2 = emery)
pebble install --emulator emery

# Voir les logs en temps réel
pebble logs --emulator emery

# Installer sur une vraie montre (Bluetooth)
pebble install --phone <IP_DU_TELEPHONE>
```

### Avant chaque publication — checklist build

- [ ] `pebble build` passe sans warning ni erreur
- [ ] Testé dans l'émulateur `emery` (200×228)
- [ ] Testé dans l'émulateur `basalt` (144×168) si `targetPlatforms` inclut basalt
- [ ] Version incrémentée dans `package.json` (`"version": "1.0.1"`)
- [ ] Aucun `hardcode` de dimensions d'écran dans le code
- [ ] Pas de `SECOND_UNIT` inutile

---

## 4. Workflow GitHub

### Un repo par watchface (recommandé)

```bash
# Init du repo
git init
git remote add origin https://github.com/<user>/<watchface-name>

# .gitignore minimal recommandé
echo "build/" >> .gitignore
echo ".pebble-linked-uuid" >> .gitignore
echo "*.pbw" >> .gitignore   # le .pbw est généré, pas versionnés
```

> **Note** : ne pas committer les `.pbw` dans Git. Ils sont générés via `pebble build` et publiés via la CLI.

### Convention de branches

```
main          → code stable, publié sur le store
dev           → développement en cours
release/x.y.z → snapshot d'une release avant publication
```

### Convention de commits

```
feat: add seconds hand animation
fix: correct layout on basalt platform
chore: bump version to 1.1.0
docs: update README with screenshot
```

### README.md — structure recommandée

```markdown
# Watchface Name

Description courte de la watchface.

## Fonctionnalités
- Heure, date, batterie...

## Plateformes supportées
- Pebble Time 2 (emery) — résolution native 200×228
- Pebble Time (basalt)

## Installation
Disponible sur le [Pebble Appstore](LIEN_APPSTORE)

## Build depuis les sources
pebble build
pebble install --emulator emery

## Licence
MIT
```

---

## 5. Publication sur le Pebble Appstore

### 5.1 Méthode recommandée — CLI (la plus directe)

```bash
# Connexion unique (ouvre le navigateur)
pebble login

# Publication interactive — génère screenshots automatiques sur toutes plateformes
pebble publish

# Ou non-interactif
pebble publish --non-interactive --description "Ma watchface pour Pebble Time 2"
```

La CLI génère automatiquement des GIFs et screenshots pour chaque plateforme et les uploade.

### 5.2 Méthode alternative — Portail développeur

URL : https://dev-portal.rebble.io

Étapes pour une watchface :
1. Connexion → clic **"Add a Watchface"**
2. Remplir : titre, URL repo GitHub (optionnel mais recommandé), email de support
3. Clic **"Create"** → page de listing
4. **"Add a release"** → uploader le `.pbw` + notes de release
5. Clic **"Save"** puis **"Publish"** sur la release
6. **"Manage Asset Collections"** → créer une collection par plateforme supportée :
   - Description (max 1600 caractères)
   - Screenshots (max 5, format PNG/GIF/GIF animé)
   - Bannière marketing (optionnel, mais utile si mis en avant)
7. Clic **"Publish"** (public) ou **"Publish Privately"** (lien direct uniquement)

> ⚠️ Une fois publié publiquement, impossible de repasser en privé sans dépublier.

### 5.3 Ressources requises pour le listing

| Ressource | Obligatoire |
|-----------|-------------|
| Titre | ✅ |
| Fichier `.pbw` | ✅ |
| Asset collection (description + 1 screenshot min) | ✅ |
| URL source code GitHub | Non (mais très recommandé) |
| Bannière marketing | Non |

### 5.4 Mettre à jour une watchface publiée

```bash
# 1. Incrémenter la version dans package.json
# 2. Rebuilder
pebble build

# 3. Publier la nouvelle version
pebble publish
```

Sur le portail : aller dans la fiche existante → **"Add a release"** → uploader le nouveau `.pbw` → **"Publish"**.

> La version dans `package.json` doit être **strictement supérieure** à la version précédemment publiée.

---

## 6. Promotion après publication

Une fois publié, partager sur :
- **Discord Pebble/Rebble** : http://rebble.io/discord
- **Reddit** : https://www.reddit.com/r/pebble
- **Bluesky** : @rebble.io
- **Mastodon** : @rebble@mastodon.social

---

## 7. Références

| Ressource | URL |
|-----------|-----|
| SDK & documentation | https://developer.rebble.io |
| Pebble Appstore | https://apps.repebble.com |
| Portail développeur | https://dev-portal.rebble.io |
| Best practices officielles | https://developer.rebble.io/guides/best-practices/ |
| Guide publication | https://developer.rebble.io/guides/appstore-publishing/publishing-an-app/ |
| Hardware info (résolutions) | https://developer.rebble.io/guides/tools-and-resources/hardware-information/ |
| Agent skill Claude Code officiel | https://github.com/coredevices/pebble-watchface-agent-skill |
