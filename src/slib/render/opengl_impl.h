/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_RENDER_OPENGL_IMPL
#define CHECKHEADER_SLIB_RENDER_OPENGL_IMPL

#include "slib/render/definition.h"

#include "slib/core/endian.h"
#include "slib/core/log.h"
#include "slib/core/thread.h"
#include "slib/core/system.h"
#include "slib/core/scoped.h"
#include "slib/graphics/image.h"

#define STACK_BUFFER_COUNT 128
#define STACK_IMAGE_SIZE 16384

#define MAX_SAMPLER_COUNT 32
#define MIN_SAMPLER_COUNT 4

#ifdef SLIB_PLATFORM_IS_TIZEN
#include <Elementary_GL_Helpers.h>
ELEMENTARY_GLVIEW_GLOBAL_DEFINE()
#endif

namespace slib
{
	void GL_BASE::setViewport(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height)
	{
		GL_ENTRY(glViewport)(x, y, width, height);
	}
	
	void GL_BASE::clear(const RenderClearParam& param)
	{
		GLbitfield mask = 0;
		if (param.flagColor) {
			GL_ENTRY(glClearColor)(param.color.getRedF(), param.color.getGreenF(), param.color.getBlueF(), param.color.getAlphaF());
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (param.flagDepth) {
#if defined(_OPENGL_IMPL)
			GL_ENTRY(glClearDepth)(param.depth);
#else
			GL_ENTRY(glClearDepthf)(param.depth);
#endif
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		if (param.flagStencil) {
			GL_ENTRY(glClearStencil)(param.stencil);
			mask |= GL_STENCIL_BUFFER_BIT;
		}
		if (mask) {
			GL_ENTRY(glClear)(mask);
		}
	}
	
	void GL_BASE::clearColor(const Color& color)
	{
		RenderClearParam param;
		param.flagColor = sl_true;
		param.color = color;
		param.flagDepth = sl_false;
		clear(param);
	}
	
	void GL_BASE::clearColorDepth(const Color& color, float depth)
	{
		RenderClearParam param;
		param.flagColor = sl_true;
		param.color = color;
		param.flagDepth = sl_true;
		param.depth = depth;
		clear(param);
	}
	
	void GL_BASE::clearDepth(float depth)
	{
		RenderClearParam param;
		param.flagColor = sl_false;
		param.flagDepth = sl_true;
		param.depth = depth;
		clear(param);
	}
	
	void GL_BASE::setDepthTest(sl_bool flag)
	{
		if (flag) {
			GL_ENTRY(glEnable)(GL_DEPTH_TEST);
		} else {
			GL_ENTRY(glDisable)(GL_DEPTH_TEST);
		}
	}
	
	void GL_BASE::setDepthWriteEnabled(sl_bool flagEnableDepthWrite)
	{
		GL_ENTRY(glDepthMask)(flagEnableDepthWrite ? GL_TRUE : GL_FALSE);
	}
	
	static GLenum _GL_getFunctionOp(RenderFunctionOperation op)
	{
		switch (op) {
			case RenderFunctionOperation::Never:
				return GL_NEVER;
			case RenderFunctionOperation::Always:
				return GL_ALWAYS;
			case RenderFunctionOperation::Equal:
				return GL_EQUAL;
			case RenderFunctionOperation::NotEqual:
				return GL_NOTEQUAL;
			case RenderFunctionOperation::Less:
				return GL_LESS;
			case RenderFunctionOperation::LessEqual:
				return GL_LEQUAL;
			case RenderFunctionOperation::Greater:
				return GL_GREATER;
			case RenderFunctionOperation::GreaterEqual:
				return GL_GEQUAL;
		}
		return GL_ALWAYS;
	}
	
	void GL_BASE::setDepthFunction(RenderFunctionOperation op)
	{
		GL_ENTRY(glDepthFunc)(_GL_getFunctionOp(op));
	}
	
	void GL_BASE::setCullFace(sl_bool flagEnableCull, sl_bool flagCullCCW)
	{
		if (flagEnableCull) {
			GL_ENTRY(glEnable)(GL_CULL_FACE);
			GL_ENTRY(glCullFace)(GL_BACK);
			if (flagCullCCW) {
				GL_ENTRY(glFrontFace)(GL_CW);
			} else {
				GL_ENTRY(glFrontFace)(GL_CCW);
			}
		} else {
			GL_ENTRY(glDisable)(GL_CULL_FACE);
		}
	}
	
	static GLenum _GL_getBlendingOp(RenderBlendingOperation op)
	{
		switch (op) {
			case RenderBlendingOperation::Add:
				return GL_FUNC_ADD;
			case RenderBlendingOperation::Subtract:
				return GL_FUNC_SUBTRACT;
			case RenderBlendingOperation::ReverseSubtract:
				return GL_FUNC_REVERSE_SUBTRACT;
		}
		return GL_FUNC_ADD;
	}
	
	static GLenum _GL_getBlendingFactor(RenderBlendingFactor factor)
	{
		switch (factor) {
			case RenderBlendingFactor::One:
				return GL_ONE;
			case RenderBlendingFactor::Zero:
				return GL_ZERO;
			case RenderBlendingFactor::SrcAlpha:
				return GL_SRC_ALPHA;
			case RenderBlendingFactor::OneMinusSrcAlpha:
				return GL_ONE_MINUS_SRC_ALPHA;
			case RenderBlendingFactor::DstAlpha:
				return GL_DST_ALPHA;
			case RenderBlendingFactor::OneMinusDstAlpha:
				return GL_ONE_MINUS_DST_ALPHA;
			case RenderBlendingFactor::SrcColor:
				return GL_SRC_COLOR;
			case RenderBlendingFactor::OneMinusSrcColor:
				return GL_ONE_MINUS_SRC_COLOR;
			case RenderBlendingFactor::DstColor:
				return GL_DST_COLOR;
			case RenderBlendingFactor::OneMinusDstColor:
				return GL_ONE_MINUS_DST_COLOR;
			case RenderBlendingFactor::SrcAlphaSaturate:
				return GL_SRC_ALPHA_SATURATE;
			case RenderBlendingFactor::Constant:
				return GL_CONSTANT_COLOR;
			case RenderBlendingFactor::OneMinusConstant:
				return GL_ONE_MINUS_CONSTANT_COLOR;
			case RenderBlendingFactor::ConstantAlpha:
				return GL_CONSTANT_ALPHA;
			case RenderBlendingFactor::OneMinusConstantAlpha:
				return GL_ONE_MINUS_CONSTANT_ALPHA;
		}
		return GL_ZERO;
	}
	
	void GL_BASE::setBlending(sl_bool flagEnableBlending, const RenderBlendingParam& param)
	{
		if (flagEnableBlending) {
			GL_ENTRY(glEnable)(GL_BLEND);
			GLenum op = _GL_getBlendingOp(param.operation);
			GLenum opAlpha = _GL_getBlendingOp(param.operationAlpha);
			if (op != opAlpha) {
				GL_ENTRY(glBlendEquationSeparate)(op, opAlpha);
			} else {
				GL_ENTRY(glBlendEquation)(op);
			}
			GLenum fSrc = _GL_getBlendingFactor(param.blendSrc);
			GLenum fDst = _GL_getBlendingFactor(param.blendDst);
			GLenum fSrcAlpha = _GL_getBlendingFactor(param.blendSrcAlpha);
			GLenum fDstAlpha = _GL_getBlendingFactor(param.blendDstAlpha);
			if (fSrc == fSrcAlpha && fDst == fDstAlpha) {
				GL_ENTRY(glBlendFunc)(fSrc, fDst);
			} else {
				GL_ENTRY(glBlendFuncSeparate)(fSrc, fDst, fSrcAlpha, fDstAlpha);
			}
			GL_ENTRY(glBlendColor)(param.blendConstant.x, param.blendConstant.y, param.blendConstant.z, param.blendConstant.w);
		} else {
			GL_ENTRY(glDisable)(GL_BLEND);
		}
	}
	
	void GL_BASE::setBlending(sl_bool flagEnableBlending)
	{
		RenderBlendingParam param;
		setBlending(flagEnableBlending, param);
	}
	
	static sl_uint32 _GL_createShader(GLenum type, const String& source)
	{
		GLuint shader = GL_ENTRY(glCreateShader)(type);
		if (shader) {
			if (source.isNotEmpty()) {
				const GLchar* sz = source.getData();
				GLint len = (GLint)(source.getLength());
				GL_ENTRY(glShaderSource)(shader, 1, &sz, &len);
				GL_ENTRY(glCompileShader)(shader);
				GLint status = 0;
				GL_ENTRY(glGetShaderiv)(shader, GL_COMPILE_STATUS, &status);
				if (status != GL_FALSE) {
					return shader;
				} else {
					GLchar log[1025];
					GLsizei lenLog = 0;
					GL_ENTRY(glGetShaderInfoLog)(shader, 1024, &lenLog, log);
					log[lenLog] = 0;
					if (type == GL_VERTEX_SHADER) {
						Log("OpenGL Compile Vertex Shader", String(log));
					} else if (type == GL_FRAGMENT_SHADER) {
						Log("OpenGL Compile Fragment Shader", String(log));
					}
				}
			}
			GL_ENTRY(glDeleteShader)(shader);
		}
		return 0;
	}
	
	sl_uint32 GL_BASE::createVertexShader(const String& source)
	{
		return _GL_createShader(GL_VERTEX_SHADER, source);
	}
	
	sl_uint32 GL_BASE::createFragmentShader(const String& source)
	{
		return _GL_createShader(GL_FRAGMENT_SHADER, source);
	}
	
	void GL_BASE::deleteShader(sl_uint32 shader)
	{
		if (shader) {
			GL_ENTRY(glDeleteShader)(shader);
		}
	}
	
	sl_uint32 GL_BASE::createProgram()
	{
		GLuint program = GL_ENTRY(glCreateProgram)();
		return program;
	}
	
	void GL_BASE::attachShader(sl_uint32 program, sl_uint32 shader)
	{
		if (program && shader) {
			GL_ENTRY(glAttachShader)(program, shader);
		}
	}
	
	sl_bool GL_BASE::linkProgram(sl_uint32 program)
	{
		if (program) {
			GL_ENTRY(glLinkProgram)(program);
			GLint status = GL_FALSE;
			GL_ENTRY(glGetProgramiv)(program, GL_LINK_STATUS, &status);
			if (status != GL_FALSE) {
				return sl_true;
			}
			GLchar log[1025];
			GLsizei lenLog = 0;
			GL_ENTRY(glGetProgramInfoLog)(program, 1024, &lenLog, log);
			log[lenLog] = 0;
			Log("OpenGL Program", String(log));
		}
		return sl_false;
	}
	
	sl_uint32 GL_BASE::createProgram(sl_uint32 vertexShader, sl_uint32 fragmentShader)
	{
		sl_uint32 program = GL_ENTRY(glCreateProgram)();
		if (program) {
			attachShader(program, vertexShader);
			attachShader(program, fragmentShader);
			if (linkProgram(program)) {
				return program;
			}
			deleteProgram(program);
		}
		return 0;
	}
	
	void GL_BASE::useProgram(sl_uint32 program)
	{
		if (program) {
			GL_ENTRY(glUseProgram)(program);
		}
	}
	
	void GL_BASE::deleteProgram(sl_uint32 program)
	{
		if (program) {
			GL_ENTRY(glDeleteProgram)(program);
		}
	}
	
	static sl_uint32 _GL_createBuffer(GLenum target, const void* data, sl_size size, sl_bool flagStatic)
	{
		GLuint buffer = 0;
		GL_ENTRY(glGenBuffers)(1, &buffer);
		if (buffer) {
			GL_ENTRY(glBindBuffer)(target, buffer);
			GL_ENTRY(glBufferData)(target, size, data, flagStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
			return buffer;
		}
		return 0;
	}
	
	static void _GL_updateBuffer(GLenum target, sl_uint32 buffer, sl_size offset, const void* data, sl_size size)
	{
		if (buffer) {
			GL_ENTRY(glBindBuffer)(target, buffer);
			GL_ENTRY(glBufferSubData)(target, buffer, size, data);
		}
	}
	
	sl_uint32 GL_BASE::createVertexBuffer(const void* data, sl_size size, sl_bool flagStatic)
	{
		return _GL_createBuffer(GL_ARRAY_BUFFER, data, size, flagStatic);
	}
	
	sl_uint32 GL_BASE::createVertexBuffer(sl_size size, sl_bool flagStatic)
	{
		return _GL_createBuffer(GL_ARRAY_BUFFER, sl_null, size, flagStatic);
	}
	
	void GL_BASE::updateVertexBuffer(sl_uint32 buffer, sl_size offset, const void* data, sl_size size)
	{
		_GL_updateBuffer(GL_ARRAY_BUFFER, buffer, offset, data, size);
	}
	
	void GL_BASE::bindVertexBuffer(sl_uint32 buffer)
	{
		GL_ENTRY(glBindBuffer)(GL_ARRAY_BUFFER, buffer);
	}
	
	void GL_BASE::unbindVertexBuffer()
	{
		GL_ENTRY(glBindBuffer)(GL_ARRAY_BUFFER, 0);
	}
	
	sl_uint32 GL_BASE::createIndexBuffer(const void* data, sl_size size, sl_bool flagStatic)
	{
		return _GL_createBuffer(GL_ELEMENT_ARRAY_BUFFER, data, size, flagStatic);
	}
	
	sl_uint32 GL_BASE::createIndexBuffer(sl_size size, sl_bool flagStatic)
	{
		return _GL_createBuffer(GL_ELEMENT_ARRAY_BUFFER, sl_null, size, flagStatic);
	}
	
	void GL_BASE::updateIndexBuffer(sl_uint32 buffer, sl_size offset, const void* data, sl_size size)
	{
		_GL_updateBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer, offset, data, size);
	}
	
	void GL_BASE::bindIndexBuffer(sl_uint32 buffer)
	{
		GL_ENTRY(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, buffer);
	}
	
	void GL_BASE::unbindIndexBuffer()
	{
		GL_ENTRY(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	void GL_BASE::deleteBuffer(sl_uint32 buffer)
	{
		if (buffer) {
			GLuint b = buffer;
			GL_ENTRY(glDeleteBuffers)(1, &b);
		}
	}
	
	sl_int32 GL_BASE::getAttributeLocation(sl_uint32 program, const char* name)
	{
		if (program) {
			GLint ret = GL_ENTRY(glGetAttribLocation)(program, name);
			return ret;
		}
		return -1;
	}
	
	static void _GL_setVertexArrayAttribute(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
	{
		if (attributeLocation != -1) {
			GL_ENTRY(glEnableVertexAttribArray)(attributeLocation);
			GL_ENTRY(glVertexAttribPointer)(attributeLocation, countComponents, GL_FLOAT, flagDoNormalize ? GL_TRUE : GL_FALSE, strideBytes, data);
		}
	}
	
#define _SL_SETVERTEXARRAY(t) \
	void GL_BASE::setVertex##t##ArrayAttributePtr(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize) \
	{ \
		_GL_setVertexArrayAttribute(attributeLocation, data, countComponents, strideBytes, flagDoNormalize); \
	} \
	void GL_BASE::setVertex##t##ArrayAttribute(sl_int32 attributeLocation, sl_size offsetValuesOnBuffer, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize) \
	{ \
		_GL_setVertexArrayAttribute(attributeLocation, (void*)offsetValuesOnBuffer, countComponents, strideBytes, flagDoNormalize); \
	}
	
	_SL_SETVERTEXARRAY(Float)
	_SL_SETVERTEXARRAY(Int8)
	_SL_SETVERTEXARRAY(Uint8)
	_SL_SETVERTEXARRAY(Int16)
	_SL_SETVERTEXARRAY(Uint16)
	
	void GL_BASE::disableVertexArrayAttribute(sl_int32 attributeLocation)
	{
		if (attributeLocation != -1) {
			GL_ENTRY(glDisableVertexAttribArray)(attributeLocation);
		}
	}
	
	sl_int32 GL_BASE::getUniformLocation(sl_uint32 program, const char* name)
	{
		if (program) {
			GLint ret = GL_ENTRY(glGetUniformLocation)(program, name);
			return ret;
		}
		return -1;
	}
	
	void GL_BASE::setUniformFloatValue(sl_int32 uniformLocation, float value)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform1f)(uniformLocation, value);
		}
	}
	
	void GL_BASE::setUniformFloatArray(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform1fv)(uniformLocation, count, (float*)values);
		}
	}
	
	void GL_BASE::setUniformIntValue(sl_int32 uniformLocation, sl_int32 value)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform1i)(uniformLocation, value);
		}
	}
	
	void GL_BASE::setUniformIntArray(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform1iv)(uniformLocation, count, (GLint*)values);
		}
	}
	
	void GL_BASE::setUniformFloat2Value(sl_int32 uniformLocation, float v1, float v2)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform2f)(uniformLocation, v1, v2);
		}
	}
	
	void GL_BASE::setUniformFloat2Value(sl_int32 uniformLocation, const Vector2& v)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform2f)(uniformLocation, v.x, v.y);
		}
	}
	
	void GL_BASE::setUniformFloat2Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform2fv)(uniformLocation, count, (float*)values);
		}
	}
	
	void GL_BASE::setUniformInt2Value(sl_int32 uniformLocation, sl_int32 v1, sl_int32 v2)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform2i)(uniformLocation, v1, v2);
		}
	}
	
	void GL_BASE::setUniformInt2Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform2iv)(uniformLocation, count, (GLint*)values);
		}
	}
	
	void GL_BASE::setUniformFloat3Value(sl_int32 uniformLocation, float v1, float v2, float v3)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform3f)(uniformLocation, v1, v2, v3);
		}
	}
	
	void GL_BASE::setUniformFloat3Value(sl_int32 uniformLocation, const Vector3& v)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform3f)(uniformLocation, v.x, v.y, v.z);
		}
	}
	
	void GL_BASE::setUniformFloat3Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform3fv)(uniformLocation, count, (float*)values);
		}
	}
	
	void GL_BASE::setUniformInt3Value(sl_int32 uniformLocation, sl_int32 v1, sl_int32 v2, sl_int32 v3)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform3i)(uniformLocation, v1, v2, v3);
		}
	}
	
	void GL_BASE::setUniformInt3Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform3iv)(uniformLocation, count, (GLint*)values);
		}
	}
	
	void GL_BASE::setUniformFloat4Value(sl_int32 uniformLocation, float v1, float v2, float v3, float v4)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform4f)(uniformLocation, v1, v2, v3, v4);
		}
	}
	
	void GL_BASE::setUniformFloat4Value(sl_int32 uniformLocation, const Vector4& v)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform4f)(uniformLocation, v.x, v.y, v.z, v.w);
		}
	}
	
	void GL_BASE::setUniformFloat4Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform4fv)(uniformLocation, count, (float*)values);
		}
	}
	
	void GL_BASE::setUniformInt4Value(sl_int32 uniformLocation, sl_int32 v1, sl_int32 v2, sl_int32 v3, sl_int32 v4)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform4i)(uniformLocation, v1, v2, v3, v4);
		}
	}
	
	void GL_BASE::setUniformInt4Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform4iv)(uniformLocation, count, (GLint*)values);
		}
	}
	
	void GL_BASE::setUniformMatrix2Value(sl_int32 uniformLocation, const Matrix2& value)
	{
		if (uniformLocation != -1) {
			float v[4];
			v[0] = value.m00;
			v[1] = value.m10;
			v[2] = value.m01;
			v[3] = value.m11;
			GL_ENTRY(glUniformMatrix2fv)(uniformLocation, 1, GL_FALSE, v);
		}
	}
	
	void GL_BASE::setUniformMatrix2Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			SLIB_SCOPED_BUFFER(float, STACK_BUFFER_COUNT, mats, 4 * count);
			for (sl_uint32 i = 0; i < count; i++) {
				float* v = mats + i * 4;
				Matrix2& value = ((Matrix2*)(values))[i];
				v[0] = value.m00;
				v[1] = value.m10;
				v[2] = value.m01;
				v[3] = value.m11;
			}
			GL_ENTRY(glUniformMatrix2fv)(uniformLocation, count, GL_FALSE, mats);
		}
	}
	
	void GL_BASE::setUniformMatrix3Value(sl_int32 uniformLocation, const Matrix3& value)
	{
		if (uniformLocation != -1) {
			float v[9];
			v[0] = value.m00;
			v[1] = value.m10;
			v[2] = value.m20;
			v[3] = value.m01;
			v[4] = value.m11;
			v[5] = value.m21;
			v[6] = value.m02;
			v[7] = value.m12;
			v[8] = value.m22;
			GL_ENTRY(glUniformMatrix3fv)(uniformLocation, 1, GL_FALSE, v);
		}
	}
	
	void GL_BASE::setUniformMatrix3Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			SLIB_SCOPED_BUFFER(float, STACK_BUFFER_COUNT, mats, 9 * count);
			for (sl_uint32 i = 0; i < count; i++) {
				float* v = mats + i * 9;
				Matrix3& value = ((Matrix3*)(values))[i];
				v[0] = value.m00;
				v[1] = value.m10;
				v[2] = value.m20;
				v[3] = value.m01;
				v[4] = value.m11;
				v[5] = value.m21;
				v[6] = value.m02;
				v[7] = value.m12;
				v[8] = value.m22;
			}
			GL_ENTRY(glUniformMatrix3fv)(uniformLocation, count, GL_FALSE, mats);
		}
	}
	
	void GL_BASE::setUniformMatrix4Value(sl_int32 uniformLocation, const Matrix4& value)
	{
		if (uniformLocation != -1) {
			float v[16];
			v[0] = value.m00;
			v[1] = value.m10;
			v[2] = value.m20;
			v[3] = value.m30;
			v[4] = value.m01;
			v[5] = value.m11;
			v[6] = value.m21;
			v[7] = value.m31;
			v[8] = value.m02;
			v[9] = value.m12;
			v[10] = value.m22;
			v[11] = value.m32;
			v[12] = value.m03;
			v[13] = value.m13;
			v[14] = value.m23;
			v[15] = value.m33;
			GL_ENTRY(glUniformMatrix4fv)(uniformLocation, 1, GL_FALSE, v);
		}
	}
	
	void GL_BASE::setUniformMatrix4Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
	{
		if (uniformLocation != -1) {
			SLIB_SCOPED_BUFFER(float, STACK_BUFFER_COUNT, mats, 16 * count);
			for (sl_uint32 i = 0; i < count; i++) {
				float* v = mats + i * 16;
				Matrix4& value = ((Matrix4*)(values))[i];
				v[0] = value.m00;
				v[1] = value.m10;
				v[2] = value.m20;
				v[3] = value.m30;
				v[4] = value.m01;
				v[5] = value.m11;
				v[6] = value.m21;
				v[7] = value.m31;
				v[8] = value.m02;
				v[9] = value.m12;
				v[10] = value.m22;
				v[11] = value.m32;
				v[12] = value.m03;
				v[13] = value.m13;
				v[14] = value.m23;
				v[15] = value.m33;
			}
			GL_ENTRY(glUniformMatrix4fv)(uniformLocation, count, GL_FALSE, mats);
		}
	}
	
	void GL_BASE::setUniformTextureSampler(sl_int32 uniformLocation, sl_uint32 textureNo)
	{
		if (uniformLocation != -1) {
			GL_ENTRY(glUniform1i)(uniformLocation, textureNo);
		}
	}
	
	void GL_BASE::setUniformTextureSamplerArray(sl_int32 uniformLocation, const sl_reg* samplers, sl_uint32 n)
	{
		if (uniformLocation != -1) {
			SLIB_SCOPED_BUFFER(GLint, 64, m, n);
			for (sl_uint32 i = 0; i < n; i++) {
				m[i] = (GLint)(samplers[i]);
			}
			GL_ENTRY(glUniform1iv)(uniformLocation, n, m);
		}
	}
	
	SLIB_INLINE static GLenum _GL_getPrimitiveType(PrimitiveType type)
	{
		switch (type) {
			case PrimitiveType::Triangle:
				return GL_TRIANGLES;
			case PrimitiveType::TriangleStrip:
				return GL_TRIANGLE_STRIP;
			case PrimitiveType::TriangleFan:
				return GL_TRIANGLE_FAN;
			case PrimitiveType::Line:
				return GL_LINES;
			case PrimitiveType::LineLoop:
				return GL_LINE_LOOP;
			case PrimitiveType::LineStrip:
				return GL_LINE_STRIP;
			case PrimitiveType::Point:
				return GL_POINTS;
		}
		return GL_TRIANGLES;
	}
	
	void GL_BASE::drawPrimitives(PrimitiveType type, sl_uint32 countVertices, sl_uint32 startIndex)
	{
		GL_ENTRY(glDrawArrays)(_GL_getPrimitiveType(type), startIndex, countVertices);
	}
	
	void GL_BASE::drawElements(PrimitiveType type, sl_uint32 countIndices, sl_size offsetBytes)
	{
		GL_ENTRY(glDrawElements)(_GL_getPrimitiveType(type), countIndices, GL_UNSIGNED_SHORT, (void*)offsetBytes);
	}
	
	void GL_BASE::setLineWidth(float width)
	{
		GL_ENTRY(glLineWidth)(width);
	}
	
	void GL_BASE::flush()
	{
		GL_ENTRY(glFlush)();
	}
	
	sl_uint32 GL_BASE::createTexture2D(const BitmapData& bitmapData)
	{
		GLuint texture = 0;
		GL_ENTRY(glGenTextures)(1, &texture);
		if (texture) {
			sl_uint32 width = bitmapData.width;
			sl_uint32 height = bitmapData.height;
			GL_ENTRY(glBindTexture)(GL_TEXTURE_2D, texture);
			if (bitmapData.format == BitmapFormat::RGBA && (bitmapData.pitch == 0 || bitmapData.pitch == (sl_int32)(width << 2))) {
				GL_ENTRY(glTexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmapData.data);
			} else {
				sl_uint32 size = width * height;
				SLIB_SCOPED_BUFFER(sl_uint8, STACK_IMAGE_SIZE, glImage, size << 2);
				BitmapData temp;
				temp.width = width;
				temp.height = height;
				temp.format = BitmapFormat::RGBA;
				temp.data = glImage;
				temp.pitch = width << 2;
				temp.copyPixelsFrom(bitmapData);
				GL_ENTRY(glTexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage);
			}
		}
		return texture;
	}
	
	sl_uint32 GL_BASE::createTexture2D(sl_uint32 width, sl_uint32 height, const Color* pixels, sl_int32 stride)
	{
		if (width > 0 && height > 0) {
			if (pixels) {
				BitmapData bitmapData(width, height, pixels, stride);
				return createTexture2D(bitmapData);
			} else {
				GLuint texture = 0;
				GL_ENTRY(glGenTextures)(1, &texture);
				if (texture) {
					GL_ENTRY(glBindTexture)(GL_TEXTURE_2D, texture);
					GL_ENTRY(glTexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sl_null);
				}
				return texture;
			}
		} else {
			return 0;
		}
	}
	
	sl_uint32 GL_BASE::createTexture2D(const Ref<Bitmap>& bitmap, sl_uint32 x, sl_uint32 y, sl_uint32 w, sl_uint32 h)
	{
		if (bitmap.isNull()) {
			return 0;
		}
		if (w == 0 || h == 0) {
			return 0;
		}
		sl_uint32 bw = bitmap->getWidth();
		sl_uint32 bh = bitmap->getHeight();
		if (bw == 0 || bh == 0) {
			return 0;
		}
		if (x >= bw) {
			return 0;
		}
		if (y >= bh) {
			return 0;
		}
		if (x + w > bw) {
			return 0;
		}
		if (y + h > bh) {
			return 0;
		}
		if (bitmap->isImage()) {
			Ref<Image> image = Ref<Image>::from(bitmap);
			return createTexture2D(w, h, image->getColors(), image->getStride());
		} else {
			SLIB_SCOPED_BUFFER(sl_uint8, STACK_IMAGE_SIZE, glImage, (w * h) << 2);
			BitmapData temp;
			temp.width = w;
			temp.height = h;
			temp.format = BitmapFormat::RGBA;
			temp.data = glImage;
			temp.pitch = w << 2;
			if (bitmap->readPixels(x, y, temp)) {
				return createTexture2D(temp);
			}
			return 0;
		}
	}
	
	sl_uint32 GL_BASE::createTexture2D(const Ref<Bitmap>& bitmap)
	{
		if (bitmap.isNull()) {
			return 0;
		}
		sl_uint32 w = bitmap->getWidth();
		sl_uint32 h = bitmap->getHeight();
		if (w == 0) {
			return 0;
		}
		if (h == 0) {
			return 0;
		}
		if (bitmap->isImage()) {
			Ref<Image> image = Ref<Image>::from(bitmap);
			return createTexture2D(w, h, image->getColors(), image->getStride());
		} else {
			SLIB_SCOPED_BUFFER(sl_uint8, STACK_IMAGE_SIZE, glImage, (w * h) << 2);
			BitmapData temp;
			temp.width = w;
			temp.height = h;
			temp.format = BitmapFormat::RGBA;
			temp.data = glImage;
			temp.pitch = w << 2;
			if (bitmap->readPixels(0, 0, temp)) {
				return createTexture2D(temp);
			}
			return 0;
		}
	}
	
	sl_uint32 GL_BASE::createTexture2DFromMemory(const void* mem, sl_size size)
	{
		Ref<Image> image = Image::loadFromMemory(mem, size);
		return createTexture2D(image);
	}
	
	void GL_BASE::updateTexture2D(sl_uint32 x, sl_uint32 y, const BitmapData& bitmapData)
	{
		sl_uint32 width = bitmapData.width;
		sl_uint32 height = bitmapData.height;
		if (bitmapData.format == BitmapFormat::RGBA && (bitmapData.pitch == 0 || bitmapData.pitch == (sl_int32)(width << 2))) {
			GL_ENTRY(glTexSubImage2D)(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bitmapData.data);
		} else {
			sl_uint32 size = width * height;
			SLIB_SCOPED_BUFFER(sl_uint8, STACK_IMAGE_SIZE, glImage, size << 2);
			BitmapData temp;
			temp.width = width;
			temp.height = height;
			temp.format = BitmapFormat::RGBA;
			temp.data = glImage;
			temp.pitch = width << 2;
			temp.copyPixelsFrom(bitmapData);
			GL_ENTRY(glTexSubImage2D)(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, glImage);
		}
	}
	
	void GL_BASE::updateTexture2D(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height, const Color* pixels, sl_int32 stride)
	{
		BitmapData bitmapData(width, height, pixels, stride);
		updateTexture2D(x, y, bitmapData);
	}
	
	void GL_BASE::updateTexture2D(sl_uint32 x, sl_uint32 y, sl_uint32 w, sl_uint32 h, const Ref<Bitmap>& bitmap, sl_uint32 bx, sl_uint32 by)
	{
		if (bitmap.isNull()) {
			return;
		}
		if (w == 0 || h == 0) {
			return;
		}
		sl_uint32 bw = bitmap->getWidth();
		sl_uint32 bh = bitmap->getHeight();
		if (bw == 0 || bh == 0) {
			return;
		}
		if (bx >= bw) {
			return;
		}
		if (by >= bh) {
			return;
		}
		if (bx + w > bw) {
			return;
		}
		if (by + h > bh) {
			return;
		}
		if (bitmap->isImage()) {
			Ref<Image> image = Ref<Image>::from(bitmap);
			updateTexture2D(x, y, w, h, image->getColorsAt(bx, by), image->getStride());
		} else {
			SLIB_SCOPED_BUFFER(sl_uint8, STACK_IMAGE_SIZE, glImage, (w * h) << 2);
			BitmapData temp;
			temp.width = w;
			temp.height = h;
			temp.format = BitmapFormat::RGBA;
			temp.data = glImage;
			temp.pitch = w << 2;
			if (bitmap->readPixels(bx, by, temp)) {
				updateTexture2D(x, y, temp);
			}
		}
	}
	
	void GL_BASE::setActiveSampler(sl_uint32 textureNo)
	{
		GL_ENTRY(glActiveTexture)(GL_TEXTURE0 + textureNo);
	}
	
	void GL_BASE::bindTexture(sl_uint32 target, sl_uint32 texture)
	{
		GL_ENTRY(glBindTexture)(target, texture);
	}
	
	void GL_BASE::unbindTexture(sl_uint32 target)
	{
		GL_ENTRY(glBindTexture)(target, 0);
	}
	
	void GL_BASE::bindTexture2D(sl_uint32 texture)
	{
		GL_ENTRY(glBindTexture)(GL_TEXTURE_2D, texture);
	}
	
	void GL_BASE::unbindTexture2D()
	{
		GL_ENTRY(glBindTexture)(GL_TEXTURE_2D, 0);
	}
	
	static GLenum _GL_getFilter(TextureFilterMode filter)
	{
		switch (filter) {
			case TextureFilterMode::Linear:
				return GL_LINEAR;
			case TextureFilterMode::Point:
				return GL_NEAREST;
		}
		return GL_NONE;
	}
	
	void GL_BASE::setTextureFilterMode(sl_uint32 target, TextureFilterMode minFilter, TextureFilterMode magFilter)
	{
		GLenum f;
		f = _GL_getFilter(minFilter);
		if (f != GL_NONE) {
			GL_ENTRY(glTexParameteri)(target, GL_TEXTURE_MIN_FILTER, f);
		}
		f = _GL_getFilter(magFilter);
		if (f != GL_NONE) {
			GL_ENTRY(glTexParameteri)(target, GL_TEXTURE_MAG_FILTER, f);
		}
	}
	
	void GL_BASE::setTexture2DFilterMode(TextureFilterMode minFilter, TextureFilterMode magFilter)
	{
		setTextureFilterMode(GL_TEXTURE_2D, minFilter, magFilter);
	}
	
	static GLenum _GL_getWrap(TextureWrapMode wrap)
	{
		switch (wrap) {
			case TextureWrapMode::Repeat:
				return GL_REPEAT;
			case TextureWrapMode::Mirror:
				return GL_MIRRORED_REPEAT;
			case TextureWrapMode::Clamp:
				return GL_CLAMP_TO_EDGE;
		}
		return GL_NONE;
	}
	
	void GL_BASE::setTextureWrapMode(sl_uint32 target, TextureWrapMode wrapX, TextureWrapMode wrapY)
	{
		GLenum f;
		f = _GL_getWrap(wrapX);
		if (f != GL_NONE) {
			GL_ENTRY(glTexParameteri)(target, GL_TEXTURE_WRAP_S, f);
		}
		f = _GL_getWrap(wrapY);
		if (f != GL_NONE) {
			GL_ENTRY(glTexParameteri)(target, GL_TEXTURE_WRAP_T, f);
		}
	}
	
	void GL_BASE::setTexture2DWrapMode(TextureWrapMode wrapX, TextureWrapMode wrapY)
	{
		setTextureWrapMode(GL_TEXTURE_2D, wrapX, wrapY);
	}
	
	void GL_BASE::deleteTexture(sl_uint32 texture)
	{
		if (texture) {
			GLuint t = texture;
			GL_ENTRY(glDeleteTextures)(1, &t);
		}
	}
	
/*****************************************
 			OpenGL Engine
******************************************/
	
	class GL_ENGINE : public GLRenderEngine
	{
	public:
		CList<sl_uint32> m_listDirtyBufferHandles;
		CList<sl_uint32> m_listDirtyTextureHandles;
		struct _ProgramHandle {
			sl_uint32 program;
			sl_uint32 vertexShader;
			sl_uint32 fragmentShader;
		};
		CList<_ProgramHandle> m_listDirtyProgramHandles;
		CList< Ref<Referable> > m_listDirtyObjects;
		
		class _RenderProgramInstance;
		Ref<RenderProgram> m_currentProgram;
		Ref<_RenderProgramInstance> m_currentProgramInstance;
		
		class _VertexBufferInstance;
		Ref<_VertexBufferInstance> m_currentVertexBufferInstance;
		
		class _IndexBufferInstance;
		Ref<_IndexBufferInstance> m_currentIndexBufferInstance;
		
		Ref<RenderProgram> m_currentProgramRendering;
		Ref<_RenderProgramInstance> m_currentProgramInstanceRendering;
		
		class _TextureInstance;
		
		class _SamplerState {
		public:
			Ref<Texture> texture;
			Ref<_TextureInstance> instance;
			TextureFilterMode minFilter;
			TextureFilterMode magFilter;
			TextureWrapMode wrapX;
			TextureWrapMode wrapY;
		public:
			void reset()
			{
				texture.setNull();
				instance.setNull();
			}
		};
		_SamplerState m_samplers[MAX_SAMPLER_COUNT];
		sl_uint32 m_nSamplers;
		sl_uint32 m_activeSampler;
		
	public:
		GL_ENGINE()
		{
			m_nSamplers = 0;
			m_activeSampler = -1;
		}
		
		~GL_ENGINE()
		{
		}
		
	public:
		void freeDirtyHandles()
		{
			{
				ListLocker<sl_uint32> list(m_listDirtyBufferHandles);
				for (sl_size i = 0; i < list.count; i++) {
					GL_BASE::deleteBuffer(list[i]);
				}
				m_listDirtyBufferHandles.removeAll_NoLock();
			}
			{
				ListLocker<sl_uint32> list(m_listDirtyTextureHandles);
				for (sl_size i = 0; i < list.count; i++) {
					GL_BASE::deleteTexture(list[i]);
				}
				m_listDirtyTextureHandles.removeAll_NoLock();
			}
			{
				ListLocker<_ProgramHandle> list(m_listDirtyProgramHandles);
				for (sl_size i = 0; i < list.count; i++) {
					GL_BASE::deleteProgram(list[i].program);
					GL_BASE::deleteShader(list[i].vertexShader);
					GL_BASE::deleteShader(list[i].fragmentShader);
				}
				m_listDirtyProgramHandles.removeAll_NoLock();
			}
			{
				m_listDirtyObjects.removeAll();
			}
		}
		
		// override
		RenderEngineType getEngineType()
		{
#if defined(_OPENGL_ES_IMPL)
			return RenderEngineType::OpenGL_ES;
#else
			return RenderEngineType::OpenGL;
#endif
		}
		
		class _RenderProgramInstance : public RenderProgramInstance
		{
		public:
			sl_uint32 vertexShader;
			sl_uint32 fragmentShader;
			sl_uint32 program;
			Ref<RenderProgramState> state;
			
		public:
			_RenderProgramInstance()
			{
				vertexShader = -1;
				fragmentShader = -1;
				program = -1;
			}
			
			~_RenderProgramInstance()
			{
				Ref<RenderEngine> engine = getEngine();
				if (engine.isNotNull()) {
					_ProgramHandle handle;
					handle.program = program;
					handle.vertexShader = vertexShader;
					handle.fragmentShader = fragmentShader;
					((GL_ENGINE*)(engine.get()))->m_listDirtyProgramHandles.add(handle);
				}
			}
			
		public:
			static Ref<_RenderProgramInstance> create(GL_ENGINE* engine, RenderProgram* program)
			{
				String vsSource = convertShader(program->getGLSLVertexShader(engine));
				String fsSource = convertShader(program->getGLSLFragmentShader(engine));
				if (vsSource.isNotEmpty() && fsSource.isNotEmpty()) {
					sl_uint32 vs = GL_BASE::createVertexShader(vsSource);
					if (vs) {
						sl_uint32 fs = GL_BASE::createFragmentShader(fsSource);
						if (fs) {
							sl_uint32 ph = GL_BASE::createProgram(vs, fs);
							if (ph) {
								Ref<RenderProgramState> state = program->onCreate(engine);
								if (state.isNotNull()) {
									state->gl_program = ph;
									state->gl_engine = engine;
									if (program->onInit(engine, state.get())) {
										Ref<_RenderProgramInstance> ret = new _RenderProgramInstance();
										if (ret.isNotNull()) {
											ret->program = ph;
											ret->vertexShader = vs;
											ret->fragmentShader = fs;
											ret->state = state;
											ret->link(engine, program);
											return ret;
										}
									}
								}
								GL_BASE::deleteProgram(ph);
							}
							GL_BASE::deleteShader(fs);
						}
						GL_BASE::deleteShader(vs);
					}
				}
				return sl_null;
			}
			
			static String convertShader(String glsl)
			{
#if defined(_OPENGL_IMPL)
#else
				if (! (glsl.contains("precision highp float;") || glsl.contains("precision mediump float;") || glsl.contains("precision lowp float;"))) {
					glsl = "precision mediump float;" + glsl;
				}
#endif
				return glsl;
			}
			
			Ref<RenderProgram> getProgram()
			{
				return Ref<RenderProgram>::from(getObject());
			}
			
		};
		
		// override
		Ref<RenderProgramInstance> _createProgramInstance(RenderProgram* program)
		{
			return _RenderProgramInstance::create(this, program);
		}
		
		class _VertexBufferInstance : public VertexBufferInstance
		{
		public:
			sl_uint32 handle;
			
		public:
			_VertexBufferInstance()
			{
				handle = -1;
			}
			
			~_VertexBufferInstance()
			{
				Ref<RenderEngine> engine = getEngine();
				if (engine.isNotNull()) {
					((GL_ENGINE*)(engine.get()))->m_listDirtyBufferHandles.add(handle);
				}
			}
			
		public:
			static Ref<_VertexBufferInstance> create(GL_ENGINE* engine, VertexBuffer* buffer)
			{
				sl_uint32 handle = GL_BASE::createVertexBuffer(buffer->getBuffer(), buffer->getSize(), buffer->isStatic());
				if (handle) {
					Ref<_VertexBufferInstance> ret = new _VertexBufferInstance();
					if (ret.isNotNull()) {
						ret->handle = handle;
						ret->link(engine, buffer);
						return ret;
					}
					GL_BASE::deleteBuffer(handle);
				}
				return sl_null;
			}
			
			// override
			void onUpdate(RenderBaseObject* object)
			{
				VertexBuffer* buffer = (VertexBuffer*)object;
				GL_BASE::updateVertexBuffer(handle, m_updatedOffset, buffer->getBuffer() + m_updatedOffset, m_updatedSize);
			}
			
		};
		
		// override
		Ref<VertexBufferInstance> _createVertexBufferInstance(VertexBuffer* buffer)
		{
			return _VertexBufferInstance::create(this, buffer);
		}
		
		class _IndexBufferInstance : public IndexBufferInstance
		{
		public:
			sl_uint32 handle;
			
		public:
			_IndexBufferInstance()
			{
				handle = -1;
			}
			
			~_IndexBufferInstance()
			{
				Ref<RenderEngine> engine = getEngine();
				if (engine.isNotNull()) {
					((GL_ENGINE*)(engine.get()))->m_listDirtyBufferHandles.add(handle);
				}
			}
			
		public:
			static Ref<_IndexBufferInstance> create(GL_ENGINE* engine, IndexBuffer* buffer)
			{
				sl_uint32 handle = GL_BASE::createIndexBuffer(buffer->getBuffer(), buffer->getSize(), buffer->isStatic());
				if (handle) {
					Ref<_IndexBufferInstance> ret = new _IndexBufferInstance();
					if (ret.isNotNull()) {
						ret->handle = handle;
						ret->link(engine, buffer);
						return ret;
					}
					GL_BASE::deleteBuffer(handle);
				}
				return sl_null;
			}
			
			// override
			void onUpdate(RenderBaseObject* object)
			{
				IndexBuffer* buffer = (IndexBuffer*)object;
				GL_BASE::updateIndexBuffer(handle, m_updatedOffset, buffer->getBuffer() + m_updatedOffset, m_updatedSize);
			}
			
		};
		
		// override
		Ref<IndexBufferInstance> _createIndexBufferInstance(IndexBuffer* buffer)
		{
			return _IndexBufferInstance::create(this, buffer);
		}
		
		class _TextureInstance : public TextureInstance
		{
		public:
			sl_uint32 handle;
			
		public:
			_TextureInstance()
			{
				handle = -1;
			}
			
			~_TextureInstance()
			{
				Ref<RenderEngine> engine = getEngine();
				if (engine.isNotNull()) {
					((GL_ENGINE*)(engine.get()))->m_listDirtyTextureHandles.add(handle);
				}
			}
			
		public:
			static Ref<_TextureInstance> create(GL_ENGINE* engine, Texture* texture)
			{
				sl_uint32 handle = GL_BASE::createTexture2D(texture->getSource());;
				if (texture->isFreeSourceOnUpdate()) {
					texture->freeSource();
				}
				if (handle) {
					Ref<_TextureInstance> ret = new _TextureInstance();
					if (ret.isNotNull()) {
						ret->handle = handle;
						ret->link(engine, texture);
						return ret;
					}
					GL_BASE::deleteTexture(handle);
				}
				return sl_null;
			}
			
			// override
			void onUpdate(RenderBaseObject* object)
			{
				Texture* texture = (Texture*)object;
				GL_BASE::bindTexture2D(handle);
				GL_BASE::updateTexture2D(m_updatedRegion.left, m_updatedRegion.top, m_updatedRegion.getWidth(), m_updatedRegion.getHeight(), texture->getSource(), m_updatedRegion.left, m_updatedRegion.top);
				if (texture->isFreeSourceOnUpdate()) {
					texture->freeSource();
				}
			}
			
		};
		
		class _NamedTexture : public EngineTexture
		{
		public:
			WeakRef<GL_ENGINE> m_engine;
			sl_uint32 m_target;
			sl_uint32 m_name;
			sl_bool m_flagDeleteOnRelease;
			
		public:
			_NamedTexture(GL_ENGINE* engine, sl_uint32 target, sl_uint32 name, sl_bool flagDeleteOnRelease):
		 m_engine(engine), m_target(target), m_name(name), m_flagDeleteOnRelease(flagDeleteOnRelease)
			{
			}
			
			~_NamedTexture()
			{
				Ref<GL_ENGINE> engine = m_engine;
				if (engine.isNotNull()) {
					if (m_flagDeleteOnRelease) {
						engine->m_listDirtyTextureHandles.add(m_name);
					}
					Ref<Referable> object = m_linkedObject;
					if (object.isNotNull()) {
						engine->m_listDirtyObjects.add(object);
					}
				}
			}
			
		};
		
		// override
		Ref<TextureInstance> _createTextureInstance(Texture* texture)
		{
			return _TextureInstance::create(this, texture);
		}
		
		// override
		sl_bool _beginScene()
		{
			freeDirtyHandles();
			return sl_true;
		}
		
		// override
		void _endScene()
		{
		}
		
		// override
		void _setViewport(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height)
		{
			GL_BASE::setViewport(x, y, width, height);
		}
		
		// override
		void _clear(const RenderClearParam& param)
		{
			GL_BASE::clear(param);
		}
		
		// override
		void _setDepthTest(sl_bool flag)
		{
			GL_BASE::setDepthTest(flag);
		}
		
		// override
		void _setDepthWriteEnabled(sl_bool flagEnableDepthWrite)
		{
			GL_BASE::setDepthWriteEnabled(flagEnableDepthWrite);
		}
		
		// override
		void _setDepthFunction(RenderFunctionOperation op)
		{
			GL_BASE::setDepthFunction(op);
		}
		
		// override
		void _setCullFace(sl_bool flagEnableCull, sl_bool flagCullCCW = sl_true)
		{
			GL_BASE::setCullFace(flagEnableCull, flagCullCCW);
		}
		
		// override
		void _setBlending(sl_bool flagEnableBlending, const RenderBlendingParam& param)
		{
			GL_BASE::setBlending(flagEnableBlending, param);
		}
		
		// override
		sl_bool _beginProgram(RenderProgram* program, RenderProgramInstance* _instance, RenderProgramState** ppState)
		{
			_RenderProgramInstance* instance = (_RenderProgramInstance*)_instance;
			if (m_currentProgramInstance != instance) {
				GL_BASE::useProgram(instance->program);
				m_currentProgramInstance = instance;
			}
			if (ppState) {
				*ppState = instance->state.get();
			}
			m_currentProgram = program;
			return sl_true;
		}
		
		// override
		void _endProgram()
		{
		}
		
		void _resetCurrentBuffers()
		{
			m_currentProgram.setNull();
			m_currentProgramInstance.setNull();
			m_currentProgramRendering.setNull();
			m_currentProgramInstanceRendering.setNull();
			m_currentVertexBufferInstance.setNull();
			m_currentIndexBufferInstance.setNull();
			for (sl_uint32 i = 0; i < m_nSamplers; i++) {
				m_samplers[i].reset();
			}
			m_activeSampler = -1;
		}
		
		// override
		void _drawPrimitive(EnginePrimitive* primitive)
		{
			if (m_currentProgram.isNull()) {
				return;
			}
			if (m_currentProgramInstance.isNull()) {
				return;
			}
			
			_VertexBufferInstance* vb = static_cast<_VertexBufferInstance*>(primitive->vertexBufferInstance.get());
			vb->_update(primitive->vertexBuffer.get());
			_IndexBufferInstance* ib = sl_null;
			if (primitive->indexBufferInstance.isNotNull()) {
				ib = (_IndexBufferInstance*)(primitive->indexBufferInstance.get());
				ib->_update(primitive->indexBuffer.get());
			}
			
			sl_bool flagResetProgramState = sl_false;
			if (m_currentProgramInstanceRendering != m_currentProgramInstance) {
				flagResetProgramState = sl_true;
			}
			if (vb != m_currentVertexBufferInstance || ib != m_currentIndexBufferInstance) {
				flagResetProgramState = sl_true;
				m_currentVertexBufferInstance = vb;
				m_currentIndexBufferInstance = ib;
				GL_BASE::bindVertexBuffer(vb->handle);
				if (ib) {
					GL_BASE::bindIndexBuffer(ib->handle);
				}
			}
			
			if (flagResetProgramState) {
				if (m_currentProgramInstanceRendering.isNotNull()) {
					m_currentProgramRendering->onPostRender(this, m_currentProgramInstanceRendering->state.get());
				}
				m_currentProgramInstanceRendering = m_currentProgramInstance;
				m_currentProgramRendering = m_currentProgram;
				m_currentProgram->onPreRender(this, m_currentProgramInstance->state.get());
			}
			
			if (ib) {
				GL_BASE::drawElements(primitive->type, primitive->countElements);
			} else {
				GL_BASE::drawPrimitives(primitive->type, primitive->countElements);
			}
		}
		
		void _initSamplersCount()
		{
			if (m_nSamplers == 0) {
				GLint n = 0;
				glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &n);
				if (n > MAX_SAMPLER_COUNT) {
					n = MAX_SAMPLER_COUNT;
				}
				if (n < MIN_SAMPLER_COUNT) {
					n = MIN_SAMPLER_COUNT;
				}
				m_nSamplers = n;
			}
		}
		
		void _setActiveSampler(sl_uint32 sampler)
		{
			if (m_activeSampler != sampler) {
				GL_BASE::setActiveSampler(sampler);
				m_activeSampler = sampler;
			}
		}
		
		// override
		void _applyTexture(Texture* texture, TextureInstance* _instance, sl_reg _samplerNo)
		{
			_initSamplersCount();
			sl_uint32 samplerNo = (sl_uint32)(_samplerNo);
			if (samplerNo >= m_nSamplers) {
				return;
			}
			if (!texture) {
				m_samplers[samplerNo].reset();
				_setActiveSampler(samplerNo);
				GL_BASE::unbindTexture2D();
				return;
			}
			if (_instance) {
				_TextureInstance* instance = (_TextureInstance*)_instance;
				if (instance->_isUpdated()) {
					_setActiveSampler(samplerNo);
					instance->_update(texture);
				}
				if (m_samplers[samplerNo].instance != instance) {
					_setActiveSampler(samplerNo);
					GL_BASE::bindTexture2D(instance->handle);
					GL_BASE::setTexture2DFilterMode(texture->getMinFilter(), texture->getMagFilter());
					GL_BASE::setTexture2DWrapMode(texture->getWrapX(), texture->getWrapY());
					m_samplers[samplerNo].texture = texture;
					m_samplers[samplerNo].instance = instance;
					m_samplers[samplerNo].minFilter = texture->getMinFilter();
					m_samplers[samplerNo].magFilter = texture->getMagFilter();
					m_samplers[samplerNo].wrapX = texture->getWrapX();
					m_samplers[samplerNo].wrapY = texture->getWrapY();
				} else {
					if (m_samplers[samplerNo].minFilter != texture->getMinFilter() || m_samplers[samplerNo].magFilter != texture->getMagFilter()) {
						_setActiveSampler(samplerNo);
						GL_BASE::setTexture2DFilterMode(texture->getMinFilter(), texture->getMagFilter());
						m_samplers[samplerNo].minFilter = texture->getMinFilter();
						m_samplers[samplerNo].magFilter = texture->getMagFilter();
					}
					if (m_samplers[samplerNo].wrapX != texture->getWrapX() || m_samplers[samplerNo].wrapY != texture->getWrapY()) {
						_setActiveSampler(samplerNo);
						GL_BASE::setTexture2DWrapMode(texture->getWrapX(), texture->getWrapY());
						m_samplers[samplerNo].wrapX = texture->getWrapX();
						m_samplers[samplerNo].wrapY = texture->getWrapY();
					}
				}
			} else {
				if (m_samplers[samplerNo].texture != texture) {
					_setActiveSampler(samplerNo);
					_NamedTexture* named = static_cast<_NamedTexture*>(texture);
					GL_BASE::bindTexture(named->m_target, named->m_name);
					GL_BASE::setTextureFilterMode(named->m_target, texture->getMinFilter(), texture->getMagFilter());
					GL_BASE::setTextureWrapMode(named->m_target, texture->getWrapX(), texture->getWrapY());
					m_samplers[samplerNo].texture = texture;
					m_samplers[samplerNo].instance.setNull();
					m_samplers[samplerNo].minFilter = texture->getMinFilter();
					m_samplers[samplerNo].magFilter = texture->getMagFilter();
					m_samplers[samplerNo].wrapX = texture->getWrapX();
					m_samplers[samplerNo].wrapY = texture->getWrapY();
				} else {
					if (m_samplers[samplerNo].minFilter != texture->getMinFilter() || m_samplers[samplerNo].magFilter != texture->getMagFilter()) {
						_setActiveSampler(samplerNo);
						GL_BASE::setTexture2DFilterMode(texture->getMinFilter(), texture->getMagFilter());
						m_samplers[samplerNo].minFilter = texture->getMinFilter();
						m_samplers[samplerNo].magFilter = texture->getMagFilter();
					}
					if (m_samplers[samplerNo].wrapX != texture->getWrapX() || m_samplers[samplerNo].wrapY != texture->getWrapY()) {
						_setActiveSampler(samplerNo);
						GL_BASE::setTexture2DWrapMode(texture->getWrapX(), texture->getWrapY());
						m_samplers[samplerNo].wrapX = texture->getWrapX();
						m_samplers[samplerNo].wrapY = texture->getWrapY();
					}
				}
			}
		}
		
		// override
		void _setLineWidth(sl_real width)
		{
			GL_BASE::setLineWidth(width);
		}
		
/*************************************************
			OpenGL entry points
**************************************************/
		
		// override
		Ref<Texture> createTextureFromName(sl_uint32 target, sl_uint32 name, sl_bool flagDeleteOnRelease)
		{
			return new _NamedTexture(this, target, name, flagDeleteOnRelease);
		}
		
		// override
		sl_int32 getAttributeLocation(sl_uint32 program, const char* name)
		{
			return GL_BASE::getAttributeLocation(program, name);
		}
		
		// override
		void setVertexFloatArrayAttributePtr(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexFloatArrayAttributePtr(attributeLocation, data, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexFloatArrayAttribute(sl_int32 attributeLocation, sl_size offsetValuesOnBuffer, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexFloatArrayAttribute(attributeLocation, offsetValuesOnBuffer, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexInt8ArrayAttributePtr(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexInt8ArrayAttributePtr(attributeLocation, data, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexInt8ArrayAttribute(sl_int32 attributeLocation, sl_size offsetValuesOnBuffer, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexInt8ArrayAttribute(attributeLocation, offsetValuesOnBuffer, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexUint8ArrayAttributePtr(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexUint8ArrayAttributePtr(attributeLocation, data, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexUint8ArrayAttribute(sl_int32 attributeLocation, sl_size offsetValuesOnBuffer, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexUint8ArrayAttribute(attributeLocation, offsetValuesOnBuffer, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexInt16ArrayAttributePtr(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexInt16ArrayAttributePtr(attributeLocation, data, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexInt16ArrayAttribute(sl_int32 attributeLocation, sl_size offsetValuesOnBuffer, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexInt16ArrayAttribute(attributeLocation, offsetValuesOnBuffer, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexUint16ArrayAttributePtr(sl_int32 attributeLocation, const void* data, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexUint16ArrayAttributePtr(attributeLocation, data, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void setVertexUint16ArrayAttribute(sl_int32 attributeLocation, sl_size offsetValuesOnBuffer, sl_uint32 countComponents, sl_uint32 strideBytes, sl_bool flagDoNormalize)
		{
			GL_BASE::setVertexUint16ArrayAttribute(attributeLocation, offsetValuesOnBuffer, countComponents, strideBytes, flagDoNormalize);
		}
		
		// override
		void disableVertexArrayAttribute(sl_int32 attributeLocation)
		{
			GL_BASE::disableVertexArrayAttribute(attributeLocation);
		}
		
		
		// override
		sl_int32 getUniformLocation(sl_uint32 program, const char* name)
		{
			return GL_BASE::getUniformLocation(program, name);
		}
		
		// override
		void setUniformFloatValue(sl_int32 uniformLocation, float value)
		{
			GL_BASE::setUniformFloatValue(uniformLocation, value);
		}
		
		// override
		void setUniformFloatArray(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformFloatArray(uniformLocation, values, count);
		}
		
		// override
		void setUniformIntValue(sl_int32 uniformLocation, sl_int32 value)
		{
			GL_BASE::setUniformIntValue(uniformLocation, value);
		}
		
		// override
		void setUniformIntArray(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformIntArray(uniformLocation, values, count);
		}
		
		// override
		void setUniformFloat2Value(sl_int32 uniformLocation, float v1, float v2)
		{
			GL_BASE::setUniformFloat2Value(uniformLocation, v1, v2);
		}
		
		// override
		void setUniformFloat2Value(sl_int32 uniformLocation, const Vector2& v)
		{
			GL_BASE::setUniformFloat2Value(uniformLocation, v);
		}
		
		// override
		void setUniformFloat2Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformFloat2Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformInt2Value(sl_int32 uniformLocation, sl_int32 v1, sl_int32 v2)
		{
			GL_BASE::setUniformInt2Value(uniformLocation, v1, v2);
		}
		
		// override
		void setUniformInt2Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformInt2Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformFloat3Value(sl_int32 uniformLocation, float v1, float v2, float v3)
		{
			GL_BASE::setUniformFloat3Value(uniformLocation, v1, v2, v3);
		}
		
		// override
		void setUniformFloat3Value(sl_int32 uniformLocation, const Vector3& v)
		{
			GL_BASE::setUniformFloat3Value(uniformLocation, v);
		}
		
		// override
		void setUniformFloat3Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformFloat3Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformInt3Value(sl_int32 uniformLocation, sl_int32 v1, sl_int32 v2, sl_int32 v3)
		{
			GL_BASE::setUniformInt3Value(uniformLocation, v1, v2, v3);
		}
		
		// override
		void setUniformInt3Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformInt3Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformFloat4Value(sl_int32 uniformLocation, float v1, float v2, float v3, float v4)
		{
			GL_BASE::setUniformFloat4Value(uniformLocation, v1, v2, v3, v4);
		}
		
		// override
		void setUniformFloat4Value(sl_int32 uniformLocation, const Vector4& v)
		{
			GL_BASE::setUniformFloat4Value(uniformLocation, v);
		}
		
		// override
		void setUniformFloat4Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformFloat4Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformInt4Value(sl_int32 uniformLocation, sl_int32 v1, sl_int32 v2, sl_int32 v3, sl_int32 v4)
		{
			GL_BASE::setUniformInt4Value(uniformLocation, v1, v2, v3, v4);
		}
		
		// override
		void setUniformInt4Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformInt4Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformMatrix2Value(sl_int32 uniformLocation, const Matrix2& value)
		{
			GL_BASE::setUniformMatrix2Value(uniformLocation, value);
		}
		
		// override
		void setUniformMatrix2Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformMatrix2Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformMatrix3Value(sl_int32 uniformLocation, const Matrix3& value)
		{
			GL_BASE::setUniformMatrix3Value(uniformLocation, value);
		}
		
		// override
		void setUniformMatrix3Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformMatrix3Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformMatrix4Value(sl_int32 uniformLocation, const Matrix4& value)
		{
			GL_BASE::setUniformMatrix4Value(uniformLocation, value);
		}
		
		// override
		void setUniformMatrix4Array(sl_int32 uniformLocation, const void* values, sl_uint32 count)
		{
			GL_BASE::setUniformMatrix4Array(uniformLocation, values, count);
		}
		
		// override
		void setUniformTextureSampler(sl_int32 uniformLocation, sl_uint32 samplerNo)
		{
			GL_BASE::setUniformTextureSampler(uniformLocation, samplerNo);
		}
		
		void setUniformTextureSamplerArray(sl_int32 uniformLocation, const sl_reg* values, sl_uint32 count)
		{
			GL_BASE::setUniformTextureSamplerArray(uniformLocation, values, count);
		}
		
	};
	
	Ref<GLRenderEngine> GL_BASE::createEngine()
	{
		return new GL_ENGINE();
	}
	
}

#endif
