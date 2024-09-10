#include "Texture.hpp"

#include <utility>

#include "Image.hpp"
#include "pch.hpp"

namespace gl {

namespace {
GLsizei GetMipLevels(int w, int h) {
  return static_cast<GLsizei>(1 + std::floor(std::log2(std::max(w, h))));
}

}  // namespace

Texture::Texture(const Tex2DCreateInfoEmpty& params) { Load(params); }
Texture::Texture(const Tex2DCreateInfo& params) { Load(params); }
Texture::Texture(const Tex2DCreateInfoLoadImage& params) { Load(params); }

Texture::Texture(Texture&& other) noexcept
    : id_(std::exchange(other.id_, 0)),
      bindless_handle_(std::exchange(other.bindless_handle_, 0)),
      resident_(std::exchange(other.resident_, false)) {}

Texture& Texture::operator=(Texture&& other) noexcept {
  this->id_ = std::exchange(other.id_, 0);
  this->bindless_handle_ = std::exchange(other.bindless_handle_, 0);
  this->resident_ = std::exchange(other.resident_, false);
  return *this;
}

Texture::~Texture() {
  if (resident_) MakeNonResident();
  if (id_) glDeleteTextures(1, &id_);
}

void Texture::Load(const TexCubeCreateParamsEmpty& params) {
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id_);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, params.wrap_s);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, params.wrap_t);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_R, params.wrap_r);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, params.min_filter);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, params.mag_filter);
  glTextureStorage2D(id_, 1, params.internal_format, params.dims.x, params.dims.y);

  if (params.gen_mipmaps) {
    glGenerateTextureMipmap(id_);
  }
}

void Texture::Load(const Tex2DCreateInfoEmpty& params) {
  ZoneScoped;
  glCreateTextures(GL_TEXTURE_2D, 1, &id_);
  glTextureStorage2D(id_, 1, params.internal_format, params.dims.x, params.dims.y);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, params.wrap_s);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, params.wrap_t);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, params.min_filter);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, params.mag_filter);
}

void Texture::Load(const Tex2DCreateInfoLoadImage& params) {
  Image img;
  if (params.type == GL_FLOAT) {
    img.LoadFromPathFloat(params.path, 0, params.flip_image);
  } else {
    img.LoadFromPath(params.path, 0, params.flip_image);
  }
  if (!img.data) return;

  glCreateTextures(GL_TEXTURE_2D, 1, &id_);
  glTextureStorage2D(id_, params.gen_mipmaps ? GetMipLevels(img.width, img.height) : 1,
                     params.internal_format, img.width, img.height);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, params.wrap_s);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, params.wrap_t);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, params.min_filter);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, params.mag_filter);
  glTextureSubImage2D(id_, 0, 0, 0, img.width, img.height, params.format, params.type, img.data);

  if (params.gen_mipmaps) {
    glGenerateTextureMipmap(id_);
  }

  if (params.bindless) {
    bindless_handle_ = glGetTextureHandleARB(id_);
    MakeResident();
  }
}

void Texture::Load(const Tex2DCreateInfo& params) {
  ZoneScoped;
  glCreateTextures(GL_TEXTURE_2D, 1, &id_);
  glTextureStorage2D(id_, params.gen_mipmaps ? GetMipLevels(params.dims.x, params.dims.y) : 1,
                     params.internal_format, params.dims.x, params.dims.y);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, params.wrap_s);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, params.wrap_t);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, params.min_filter);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, params.mag_filter);
  glTextureSubImage2D(id_, 0, 0, 0, params.dims.x, params.dims.y, params.format, params.type,
                      params.data);

  if (params.gen_mipmaps) {
    glGenerateTextureMipmap(id_);
  }

  if (params.bindless) {
    bindless_handle_ = glGetTextureHandleARB(id_);
    MakeResident();
  }
}

void Texture::Bind(int unit) const { glBindTextureUnit(unit, id_); }

void Texture::MakeNonResident() {
  EASSERT_MSG(resident_, "Must be resident in order to call make not resident");
  glMakeTextureHandleNonResidentARB(bindless_handle_);
  resident_ = false;
}

void Texture::MakeResident() {
  EASSERT_MSG(!resident_, "Cannot already be resident.");
  glMakeTextureHandleResidentARB(bindless_handle_);
  resident_ = true;
}

}  // namespace gl
