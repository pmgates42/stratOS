---
description: "StratOS Unity test-app structure and test_apps/apps.json profile conventions"
applyTo: "{apps/**/*_test*.c,test_apps/**,test/app_test/**}"
---
- Test apps use Unity via `app_test.h`: define `APP_test_setup`/`APP_test_teardown`, static `test_<behavior_in_words>` functions, `APP_<name>_test_configure()` and `APP_<name>_test_run()` (using `APP_test_begin`/`APP_test_run`/`APP_test_end`), and export `boolean APP_sched_configure(void)`.
- Profiles in `test_apps/apps.json` set `"test_framework": "app_test"` and `-DUNITY_EXCLUDE_STDINT_H` in each platform's cflags; `bcm2xxx` also needs the `UNITY_EXCLUDE_*` and `UNITY_OUTPUT_CHAR` flags (copy from `sched_rate_test`).
- Host-only unit tests live in `test/unit/`.
