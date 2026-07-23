package com.multirium.mobile.gui.keyboard

import kotlin.jvm.internal.DefaultConstructorMarker
import kotlin.jvm.internal.Intrinsics

class CommandInfo(str: String, str2: String, str3: String, z: Boolean, i: Int) {
    val adminLevel: Int
    val description: String
    val isAdmin: Boolean
    val name: String
    val props: String

    fun component1(): String {
        return this.name
    }

    fun component2(): String {
        return this.description
    }

    fun component3(): String {
        return this.props
    }

    fun component4(): Boolean {
        return this.isAdmin
    }

    fun component5(): Int {
        return this.adminLevel
    }

    fun copy(str: String, str2: String, str3: String, z: Boolean, i: Int): CommandInfo {
        Intrinsics.checkNotNullParameter(str, "name")
        Intrinsics.checkNotNullParameter(str2, "description")
        Intrinsics.checkNotNullParameter(str3, "props")
        return CommandInfo(str, str2, str3, z, i)
    }

    override fun equals(obj: Any?): Boolean {
        if (this === obj) {
            return true
        }
        if (obj !is CommandInfo) {
            return false
        }
        val commandInfo = obj
        return Intrinsics.areEqual(name as Any, commandInfo.name as Any) && Intrinsics.areEqual(
            description as Any, commandInfo.description as Any
        ) && Intrinsics.areEqual(
            props as Any, commandInfo.props as Any
        ) && this.isAdmin == commandInfo.isAdmin && this.adminLevel == commandInfo.adminLevel
    }

    override fun hashCode(): Int {
        val hashCode =
            ((((name.hashCode() * 31) + description.hashCode()) * 31) + props.hashCode()) * 31
        var z = this.isAdmin
        if (z) {
            z = true
        }
        return ((hashCode + (if (z) 1 else 0)) * 31) + Integer.hashCode(this.adminLevel)
    }

    override fun toString(): String {
        return "CommandInfo(name=" + this.name + ", description=" + this.description + ", props=" + this.props + ", isAdmin=" + this.isAdmin + ", adminLevel=" + this.adminLevel + ')'
    }

    init {
        Intrinsics.checkNotNullParameter(str, "name")
        Intrinsics.checkNotNullParameter(str2, "description")
        Intrinsics.checkNotNullParameter(str3, "props")
        this.name = str
        this.description = str2
        this.props = str3
        this.isAdmin = z
        this.adminLevel = i
    }

    constructor(
        str: String,
        str2: String,
        str3: String?,
        z: Boolean,
        i: Int,
        i2: Int,
        defaultConstructorMarker: DefaultConstructorMarker?
    ) : this(
        str,
        str2,
        (if ((i2 and 4) != 0) "" else str3)!!,
        if ((i2 and 8) != 0) false else z,
        if ((i2 and 16) != 0) 0 else i
    )

    companion object {
        const val `$stable`: Int = 0
        fun `copy$default`(
            commandInfo: CommandInfo,
            str: String,
            str2: String,
            str3: String,
            z: Boolean,
            i: Int,
            i2: Int,
            obj: Any?
        ): CommandInfo {
            var str = str
            var str2 = str2
            var str3 = str3
            var z = z
            var i = i
            if ((i2 and 1) != 0) {
                str = commandInfo.name
            }
            if ((i2 and 2) != 0) {
                str2 = commandInfo.description
            }
            val str4 = str2
            if ((i2 and 4) != 0) {
                str3 = commandInfo.props
            }
            val str5 = str3
            if ((i2 and 8) != 0) {
                z = commandInfo.isAdmin
            }
            val z2 = z
            if ((i2 and 16) != 0) {
                i = commandInfo.adminLevel
            }
            return commandInfo.copy(str, str4, str5, z2, i)
        }
    }
}
