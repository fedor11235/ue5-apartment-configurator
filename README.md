# Apartment Configurator — Unreal Engine 5

Интерактивный конфигуратор этажей и квартир здания. Данные (этажи, квартиры, статус,
площадь, координаты фокуса камеры) загружаются из локального JSON-файла; камера имеет три
режима навигации, интерфейс на UMG строится динамически, квартиры выбираются кликом в 3D-сцене.

> **Тестовое задание (UE5 Developer, Middle+).** Движок: **Unreal Engine 5.7–5.8**,
> языки — **C++ + Blueprints**, формат данных — **JSON**.

---

## ⚠️ Что в этом репозитории есть, а чего нет

Этот репозиторий — **C++-каркас проекта** (вся логика архитектуры: загрузка данных, камера,
взаимодействие, UI-классы). Он задаёт модульную структуру, в которой C++ держит поведение,
а визуальная часть (сцена, 3D-модель здания, дизайн UMG-виджетов, Blueprint-наследники)
достраивается в редакторе Unreal.

**Есть (в `Source/` + `Config/`):**
- вся C++-логика: subsystem загрузки JSON, контроллер камеры, кликабельные акторы квартир,
  базовые классы UMG-виджетов, player controller-оркестратор, game mode;
- пример данных `Config/BuildingConfig.json`;
- `.uproject`, `Build.cs`, `Target.cs`, конфиги проекта.

**Нет (создаётся в редакторе — бинарные `.uasset`/`.umap`, в git не коммитятся):**
- уровень `.umap` со сценой и расставленными акторами квартир;
- 3D-модель/меши здания и материал с параметрами `BaseColor` / `Opacity`;
- Blueprint-наследники C++-классов (`BP_ApartmentActor`, `WBP_FloorPanel`, `WBP_ApartmentCard`,
  `WBP_FloorButton`, `WBP_HUD`) и дизайн виджетов;
- демонстрационное видео 1–2 мин.

Раздел [«Сборка в редакторе»](#сборка-в-редакторе-шаги-художникаdevа) ниже — точная инструкция,
как довести каркас до запускаемого проекта.

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
├─ UI/            UMG (базовые C++-классы)
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

## Сборка в редакторе (шаги художника/dev-а)

1. **Генерация проектных файлов.** ПКМ по `ApartmentConfigurator.uproject` → *Generate Visual
   Studio / Xcode project files*; собрать C++-модуль (цель `ApartmentConfiguratorEditor`).
2. **Материал.** Создать `M_Apartment` с векторным параметром `BaseColor` и скалярным `Opacity`
   (translucent/masked), назначить на меш квартиры.
3. **Blueprint-наследники C++-классов:**
   - `BP_ApartmentActor` ← `AApartmentActor` (задать меш + материал-слот 0);
   - `WBP_FloorButton` ← `UFloorButtonWidget` (привязать `FloorButton`, `FloorLabel`);
   - `WBP_FloorPanel` ← `UFloorPanelWidget` (привязать `FloorButtonContainer`, `HideSoldCheckBox`;
     задать `FloorButtonClass = WBP_FloorButton`);
   - `WBP_ApartmentCard` ← `UApartmentCardWidget` (привязать `IdText`,`AreaText`,`StatusText`,`BookButton`,`CloseButton`);
   - `WBP_HUD` ← `UConfiguratorHUDWidget` (вложить `WBP_FloorPanel`, `WBP_ApartmentCard`; привязать `BackButton`);
   - `BP_ConfiguratorPlayerController` ← `AConfiguratorPlayerController` (задать `HUDWidgetClass = WBP_HUD`);
   - `BP_ConfiguratorGameMode` ← `AConfiguratorGameMode` (задать PlayerController = `BP_ConfiguratorPlayerController`).
4. **Сцена.** Создать уровень, поставить модель здания, добавить `AConfiguratorCameraController`,
   расставить `BP_ApartmentActor` по квартирам и выставить каждому `id`, совпадающий с JSON.
   Выставить `GlobalDefaultGameMode = BP_ConfiguratorGameMode` (или оставить C++-класс из
   `DefaultEngine.ini`).
5. **Данные.** При необходимости поправить `Config/BuildingConfig.json`.
6. **Запуск.** Play In Editor: Genplan → клик по этажу → клик по квартире → карточка → «Назад».

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
