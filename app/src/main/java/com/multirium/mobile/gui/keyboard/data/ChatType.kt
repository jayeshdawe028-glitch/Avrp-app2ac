package com.multirium.mobile.gui.keyboard.data

import kotlin.jvm.internal.Intrinsics

class ChatType(str: String, str2: String) {
    val command: String
    val title: String

    fun component1(): String {
        return this.title
    }

    fun component2(): String {
        return this.command
    }

    fun copy(str: String, str2: String): ChatType {
        Intrinsics.checkNotNullParameter(str, "title")
        Intrinsics.checkNotNullParameter(str2, "command")
        return ChatType(str, str2)
    }

    override fun equals(obj: Any?): Boolean {
        if (this === obj) {
            return true
        }
        if (obj !is ChatType) {
            return false
        }
        val chatType = obj
        return Intrinsics.areEqual(title as Any, chatType.title as Any) && Intrinsics.areEqual(
            command as Any, chatType.command as Any
        )
    }

    override fun hashCode(): Int {
        return (title.hashCode() * 31) + command.hashCode()
    }

    override fun toString(): String {
        return "ChatType(title=" + this.title + ", command=" + this.command + ')'
    }

    init {
        Intrinsics.checkNotNullParameter(str, "title")
        Intrinsics.checkNotNullParameter(str2, "command")
        this.title = str
        this.command = str2
    }

    companion object {
        const val `$stable`: Int = 0
        fun `copy$default`(
            chatType: ChatType,
            str: String,
            str2: String,
            i: Int,
            obj: Any?
        ): ChatType {
            var str = str
            var str2 = str2
            if ((i and 1) != 0) {
                str = chatType.title
            }
            if ((i and 2) != 0) {
                str2 = chatType.command
            }
            return chatType.copy(str, str2)
        }
    }
}
