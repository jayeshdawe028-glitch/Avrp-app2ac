#-dontshrink
#-keepclasseswithmembernames class com.nvidia.devtech.*, com.wardrumstudios.utils.*

-keep class com.nvidia.devtech.* { *; }

-keep class com.wardrumstudios.utils.* { *; }

-keep class com.multirium.mobile.* { *; }
-keep class com.multirium.mobile.gui.* { *; }
-keep class com.multirium.mobile.gui.main_menu.utils.SettingMenuUtil { *; }
#-keep class com.multirium.mobile.gui.dialogs.Dialog { *; }

-dontwarn javax.servlet.**
-dontwarn org.conscrypt.**
-dontwarn org.bouncycastle.**
-dontwarn org.openjsse.**
-keep class com.google.firebase.messaging.** { *; }
-keep class com.google.firebase.** { *; }
-keep class com.google.firebase.messaging.** { *; }
-keep class com.google.firebase.crashlytics.** { *; }
-keep class com.google.firebase.analytics.** { *; }
-dontwarn com.google.firebase.**
