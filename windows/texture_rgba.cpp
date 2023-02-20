#include "texture_rgba.h"
#include <iostream>

TextureRgba::TextureRgba(flutter::TextureRegistrar *texture_registrar)
{
	this->texture_registrar_ = texture_registrar;
	// TODO: flutter only support PixelBuffer in 2023.02.17.
	auto buffer = flutter::PixelBufferTexture([=](size_t width, size_t height) -> const FlutterDesktopPixelBuffer *
											  { return this->buffer(); });
	texture_ = std::make_unique<flutter::TextureVariant>(buffer);
	this->texture_id_ = texture_registrar->RegisterTexture(texture_.get());
}

TextureRgba::~TextureRgba()
{
	texture_registrar_->UnregisterTexture(texture_id_);
}

void TextureRgba::MarkVideoFrameAvailable(
	std::vector<uint8_t> &buffer, size_t width, size_t height)
{
	const std::lock_guard<std::mutex> lock(mutex_);
	if (last_fg_index_ != fg_index_)
	{
		flutter_pixel_buffer_.buffer = static_cast<const uint8_t *>(buffer_tmp_[fg_index_].data());
		flutter_pixel_buffer_.width = width_[fg_index_];
		flutter_pixel_buffer_.height = height_[fg_index_];
		last_fg_index_ = fg_index_;
		texture_registrar_->MarkTextureFrameAvailable(texture_id_);
	}

	int bg_index = fg_index_ ^ 1;
	buffer.swap(buffer_tmp_[bg_index]);
	width_[bg_index] = width;
	height_[bg_index] = height;
}

inline const FlutterDesktopPixelBuffer *TextureRgba::buffer()
{
	const std::lock_guard<std::mutex> lock(mutex_);
	fg_index_ ^= 1;
	return &this->flutter_pixel_buffer_;
}
