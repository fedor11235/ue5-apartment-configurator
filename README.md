# Apartment Configurator — Unreal Engine 5

Интерактивный конфигуратор этажей и квартир здания. Данные (этажи, квартиры, статус,
площадь, координаты фокуса камеры) загружаются из локального JSON-файла; камера имеет три
режима навигации, интерфейс на UMG строится динамически, квартиры выбираются кликом в 3D-сцене.

> **Тестовое задание (UE5 Developer, Middle+).** Движок: **Unreal Engine 5.7–5.8**,
> языки — **C++ + Blueprints**, формат данных — **JSON**.

---

## Что в этом репозитории

Полностью **запускаемый проект**: C++-логика + готовый уровень со сценой. Открыть и нажать Play.

- Вся C++-логика: subsystem загрузки JSON, контроллер камеры, кликабельные акторы квартир,
  UMG-виджеты, player controller-оркестратор, game mode.
- **UI строится целиком в C++** (`RebuildWidget()` в каждом виджете) — без WBP-дизайн-ассетов и
  без `BindWidget`. Любой виджет можно при желании переопределить Blueprint-наследником, но для
  запуска это не требуется.
- Готовый контент (генерируется скриптом `Scripts/gen_configurator.py`, в репозитории закоммичен):
  - `Content/Maps/L_Configurator.umap` — сцена: свет, пол, орбитальная камера и 5 акторов квартир,
    расставленных по координатам из JSON, каждому выставлен `ApartmentId`;
  - `Content/Materials/M_Apartment.uasset` — unlit-материал с параметрами `BaseColor` (вектор) и
    `Opacity` (скаляр), которые актор гоняет через динамический material instance.
- Пример данных `Config/BuildingConfig.json`; `.uproject`, `Build.cs`, `Target.cs`, конфиги.

**Остаётся сделать вручную (вне кода):** записать демонстрационное видео 1–2 мин.

> Контент можно пересоздать из кода в любой момент:
> ```
> UnrealEditor-Cmd ApartmentConfigurator.uproject -run=pythonscript \
>   -script="Scripts/gen_configurator.py" -unattended -nullrhi
> ```

---

## Архитектура

Код разбит на 4 независимых слоя. Каждый слой знает только свою задачу и общается через
делегаты; единственная точка, где всё связывается, — `AConfiguratorPlayerController`
(«директор»). Это облегчает тестирование и замену любой части.

```
Source/ApartmentConfigurator/
├─ Data/          загрузка и разбор данных
│  ├─ ConfiguratorTypes.h          FBuildingConfig / FFloorData / FApartmentData / EApartmentStatus
│  └─ BuildingConfigSubsystem.*     UGameInstanceSubsystem: async-загрузка + защитный парсинг JSON
├─ Camera/        навигация камеры
│  └─ ConfiguratorCameraController.*  3 режима, плавная интерполяция, стек «назад», орбита
├─ Interaction/   3D-взаимодействие
│  └─ ApartmentActor.*              кликабельный актор квартиры: подсветка, статус, фильтр
├─ UI/            UMG (виджеты строятся целиком в C++ через RebuildWidget)
│  ├─ FloorButtonWidget.*           одна кнопка этажа
│  ├─ FloorPanelWidget.*            панель этажей (генерация кнопок) + чекбокс «скрыть проданные»
│  ├─ ApartmentCardWidget.*         карточка квартиры: ID / площадь / статус / «Забронировать»
│  └─ ConfiguratorHUDWidget.*       корневой экран: панель + карточка + кнопка «Назад»
└─ Player/        оркестрация
   ├─ ConfiguratorPlayerController.*  связывает данные ↔ камеру ↔ акторы ↔ UI; клик-детект
   └─ ConfiguratorGameMode.*          дефолтные классы
```

### Ключевые архитектурные решения

- **Данные как `GameInstanceSubsystem`.** Конфиг переживает смену уровней и доступен из любого
  `UObject` через `GetGameInstance()->GetSubsystem<UBuildingConfigSubsystem>()` — без синглтонов
  и без хранения данных в акторах.
- **Неблокирующая загрузка.** Чтение файла и разбор идут в фоновом потоке
  (`AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, …)`), результат маршалится обратно в
  игровой поток и рассылается делегатом `OnConfigLoaded`. Игровой поток не стопорится на IO.
- **Защитный парсинг (устойчивость к битому JSON — оценочный критерий).** Разбор полностью
  ручной на `FJsonObject` с `TryGet*`-проверками: отсутствующее/переименованное/неверно-типизированное
  поле даёт предупреждение в лог и разумный дефолт, а не креш. Пустой конфиг трактуется как ошибка
  загрузки, UI остаётся в безопасном состоянии. `TWeakObjectPtr` защищает от обращения к
  уничтоженному subsystem (например, остановка PIE во время загрузки).
- **Камера: цель + интерполяция.** Контроллер хранит целевой трансформ и каждый тик плавно
  интерполирует к нему (`VInterpTo`/`RInterpTo`, кадронезависимо). Переходы между всеми режимами
  плавные по построению. История видов — стек `ViewStack`, `GoBack()` возвращает предыдущий вид.
- **Три режима камеры** (`EConfiguratorCameraMode`): `Genplan` (здание целиком + свободная орбита
  по драгу мыши), `Floor` (фокус на этаже по `focusLocation` из JSON), `Apartment` (приближение к
  квартире по `focusLocation`/`focusRotation` из JSON).
- **Взаимодействие.** `bEnableClickEvents` на player controller + `OnClicked` на меше актора.
  Проданные/скрытые фильтром квартиры не кликабельны (`bInteractable=false`) и визуально отличаются
  через динамический материал (`BaseColor`, `Opacity`).
- **UI развязан от логики.** Виджеты только отображают данные и поднимают делегаты
  (`OnFloorSelected`, `OnHideSoldChanged`, `OnBookClicked`, `OnBackRequested`). Реакцию на них
  задаёт player controller.
- **UI целиком в коде.** Каждый виджет строит своё дерево в `RebuildWidget()`
  (`WidgetTree->ConstructWidget<…>`). Это убирает зависимость от ручной вёрстки WBP и `BindWidget`:
  проект запускается «из коробки», а дизайн при желании переопределяется Blueprint-наследником.
- **Связывание акторов с данными по `ApartmentId`.** У `AApartmentActor` есть редактируемое поле
  `ApartmentId`; на старте player controller находит все акторы на уровне и сопоставляет их с
  записями конфига по этому id (`BindApartmentActors`).

---

## Формат данных — `Config/BuildingConfig.json`

```jsonc
{
  "buildingName": "Riverside Residence",
  "floors": [
    {
      "floorNumber": 1,
      "displayName": "Ground Floor",
      "focusLocation": { "x": 0, "y": -1200, "z": 300 },   // камера для режима Floor
      "apartments": [
        {
          "id": "A-101",                                    // обязательное; без id квартира отбрасывается
          "status": "Available",                            // "Available" | "Sold" (регистронезависимо)
          "area": 54.3,                                     // м²
          "focusLocation": { "x": -400, "y": -600, "z": 150 },        // камера для режима Apartment
          "focusRotation": { "pitch": -10, "yaw": 90, "roll": 0 }
        }
      ]
    }
  ]
}
```

Любое поле, кроме `apartments[].id`, необязательно и имеет дефолт. `floors` обязателен и непуст,
иначе конфиг считается невалидным.

---

## Запуск

1. **Собрать C++** (один раз). Цель `ApartmentConfiguratorEditor`:
   ```
   "<UE>/Engine/Build/BatchFiles/Mac/Build.sh" ApartmentConfiguratorEditor Mac Development \
     -Project="ApartmentConfigurator.uproject" -WaitMutex
   ```
   (на Windows — `Build.bat`). Либо открыть `.uproject` и согласиться на сборку модуля.
2. **Открыть** `ApartmentConfigurator.uproject` — стартовая карта `L_Configurator` уже прописана
   в `DefaultEngine.ini`.
3. **Play In Editor.** Сценарий: Genplan (драг мыши — орбита вокруг здания) → клик по кнопке
   этажа слева → клик по квартире-боксу в сцене → открывается карточка (ID / площадь / статус /
   «Book») → «< Back». Чекбокс «Hide sold» затемняет и выключает проданные квартиры.
4. **Данные.** Правка `Config/BuildingConfig.json` меняет этажи/квартиры. Если в сцене нужны
   акторы под новые id — перегенерировать уровень скриптом `Scripts/gen_configurator.py`
   (см. выше) или расставить `AApartmentActor` вручную, выставив им `ApartmentId`.

GameMode задаётся C++-классом `AConfiguratorGameMode` через `GlobalDefaultGameMode` в
`DefaultEngine.ini`; player controller по умолчанию поднимает C++-HUD (`HUDWidgetClass`).

---

## Проверка критериев оценки

| Критерий ТЗ | Где в коде |
|---|---|
| Модуль/subsystem загрузки JSON | `Data/BuildingConfigSubsystem.*` |
| Устойчивость к битому/неполному JSON | `ParseConfig` / `Parse*` — `TryGet*` + дефолты + лог |
| Неблокирующая загрузка | `LoadConfigAsync` — фоновый `AsyncTask` + маршалинг в game thread |
| 3 режима камеры | `Camera/ConfiguratorCameraController.*`, `EConfiguratorCameraMode` |
| Плавные переходы + «назад» | `Tick` (`*InterpTo`) + `ViewStack`/`GoBack()` |
| Динамическая панель этажей | `UI/FloorPanelWidget::BuildFromConfig` |
| Карточка квартиры + «Забронировать» | `UI/ApartmentCardWidget` (`BookButton` отключается для Sold) |
| Чекбокс «скрыть проданные» + отличие в 3D | `FloorPanelWidget` → `HandleHideSoldChanged` → `ApartmentActor::SetFilteredOut` |
| Клик по квартире → фокус + карточка | `ApartmentActor::HandleMeshClicked` → `PlayerController::FocusApartment` |
| Чистые Saved/Intermediate | `.gitignore` |
| Комментированный C++ | исходники в `Source/` |

---

*Движок: Unreal Engine 5.7–5.8 · C++ + Blueprints*
