// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/font_render_params.h"

#include "base/files/scoped_temp_dir.h"
#include "base/logging.h"
#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/font.h"
#include "ui/gfx/linux_font_delegate.h"
#include "ui/gfx/test/fontconfig_util_linux.h"

namespace gfx {

namespace {

// Implementation of LinuxFontDelegate that returns a canned FontRenderParams
// struct. This is used to isolate tests from the system's local configuration.
class TestFontDelegate : public LinuxFontDelegate {
 public:
  TestFontDelegate() {}
  ~TestFontDelegate() override {}

  void set_params(const FontRenderParams& params) { params_ = params; }

  FontRenderParams GetDefaultFontRenderParams() const override {
    return params_;
  }
  void GetDefaultFontDescription(
      std::string* family_out,
      int* size_pixels_out,
      int* style_out,
      FontRenderParams* params_out) const override {
    NOTIMPLEMENTED();
  }

 private:
  FontRenderParams params_;

  DISALLOW_COPY_AND_ASSIGN(TestFontDelegate);
};

}  // namespace

class FontRenderParamsTest : public testing::Test {
 public:
  FontRenderParamsTest() {
    SetUpFontconfig();
    CHECK(temp_dir_.CreateUniqueTempDir());
    original_font_delegate_ = LinuxFontDelegate::instance();
    LinuxFontDelegate::SetInstance(&test_font_delegate_);
    ClearFontRenderParamsCacheForTest();
  }

  ~FontRenderParamsTest() override {
    LinuxFontDelegate::SetInstance(
        const_cast<LinuxFontDelegate*>(original_font_delegate_));
    TearDownFontconfig();
  }

 protected:
  base::ScopedTempDir temp_dir_;
  const LinuxFontDelegate* original_font_delegate_;
  TestFontDelegate test_font_delegate_;

 private:
  DISALLOW_COPY_AND_ASSIGN(FontRenderParamsTest);
};

TEST_F(FontRenderParamsTest, Default) {
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("arial.ttf"));
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      // Specify the desired defaults via a font match rather than a pattern
      // match (since this is the style generally used in /etc/fonts/conf.d).
      kFontconfigMatchFontHeader +
      CreateFontconfigEditStanza("antialias", "bool", "true") +
      CreateFontconfigEditStanza("autohint", "bool", "true") +
      CreateFontconfigEditStanza("hinting", "bool", "true") +
      CreateFontconfigEditStanza("hintstyle", "const", "hintslight") +
      CreateFontconfigEditStanza("rgba", "const", "rgb") +
      kFontconfigMatchFooter +
      // Add a font match for Arial. Since it specifies a family, it shouldn't
      // take effect when querying default settings.
      kFontconfigMatchFontHeader +
      CreateFontconfigTestStanza("family", "eq", "string", "Arial") +
      CreateFontconfigEditStanza("antialias", "bool", "true") +
      CreateFontconfigEditStanza("autohint", "bool", "false") +
      CreateFontconfigEditStanza("hinting", "bool", "true") +
      CreateFontconfigEditStanza("hintstyle", "const", "hintfull") +
      CreateFontconfigEditStanza("rgba", "const", "none") +
      kFontconfigMatchFooter +
      // Add font matches for fonts between 10 and 20 points or pixels. Since
      // they specify sizes, they also should not affect the defaults.
      kFontconfigMatchFontHeader +
      CreateFontconfigTestStanza("size", "more_eq", "double", "10.0") +
      CreateFontconfigTestStanza("size", "less_eq", "double", "20.0") +
      CreateFontconfigEditStanza("antialias", "bool", "false") +
      kFontconfigMatchFooter +
      kFontconfigMatchFontHeader +
      CreateFontconfigTestStanza("pixel_size", "more_eq", "double", "10.0") +
      CreateFontconfigTestStanza("pixel_size", "less_eq", "double", "20.0") +
      CreateFontconfigEditStanza("antialias", "bool", "false") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  FontRenderParams params = GetFontRenderParams(
      FontRenderParamsQuery(true), NULL);
  EXPECT_TRUE(params.antialiasing);
  EXPECT_TRUE(params.autohinter);
  EXPECT_TRUE(params.use_bitmaps);
  EXPECT_EQ(FontRenderParams::HINTING_SLIGHT, params.hinting);
  EXPECT_FALSE(params.subpixel_positioning);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_RGB,
            params.subpixel_rendering);
}

TEST_F(FontRenderParamsTest, Size) {
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("arial.ttf"));
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      kFontconfigMatchPatternHeader +
      CreateFontconfigEditStanza("antialias", "bool", "true") +
      CreateFontconfigEditStanza("hinting", "bool", "true") +
      CreateFontconfigEditStanza("hintstyle", "const", "hintfull") +
      CreateFontconfigEditStanza("rgba", "const", "none") +
      kFontconfigMatchFooter +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("pixelsize", "less_eq", "double", "10") +
      CreateFontconfigEditStanza("antialias", "bool", "false") +
      kFontconfigMatchFooter +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("size", "more_eq", "double", "20") +
      CreateFontconfigEditStanza("hintstyle", "const", "hintslight") +
      CreateFontconfigEditStanza("rgba", "const", "rgb") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  // The defaults should be used when the supplied size isn't matched by the
  // second or third blocks.
  FontRenderParamsQuery query(false);
  query.pixel_size = 12;
  FontRenderParams params = GetFontRenderParams(query, NULL);
  EXPECT_TRUE(params.antialiasing);
  EXPECT_EQ(FontRenderParams::HINTING_FULL, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_NONE,
            params.subpixel_rendering);

  query.pixel_size = 10;
  params = GetFontRenderParams(query, NULL);
  EXPECT_FALSE(params.antialiasing);
  EXPECT_EQ(FontRenderParams::HINTING_FULL, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_NONE,
            params.subpixel_rendering);

  query.pixel_size = 0;
  query.point_size = 20;
  params = GetFontRenderParams(query, NULL);
  EXPECT_TRUE(params.antialiasing);
  EXPECT_EQ(FontRenderParams::HINTING_SLIGHT, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_RGB,
            params.subpixel_rendering);
}

TEST_F(FontRenderParamsTest, Style) {
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("arial.ttf"));
  // Load a config that disables subpixel rendering for bold text and disables
  // hinting for italic text.
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      kFontconfigMatchPatternHeader +
      CreateFontconfigEditStanza("antialias", "bool", "true") +
      CreateFontconfigEditStanza("hinting", "bool", "true") +
      CreateFontconfigEditStanza("hintstyle", "const", "hintslight") +
      CreateFontconfigEditStanza("rgba", "const", "rgb") +
      kFontconfigMatchFooter +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("weight", "eq", "const", "bold") +
      CreateFontconfigEditStanza("rgba", "const", "none") +
      kFontconfigMatchFooter +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("slant", "eq", "const", "italic") +
      CreateFontconfigEditStanza("hinting", "bool", "false") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  FontRenderParamsQuery query(false);
  query.style = Font::NORMAL;
  FontRenderParams params = GetFontRenderParams(query, NULL);
  EXPECT_EQ(FontRenderParams::HINTING_SLIGHT, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_RGB,
            params.subpixel_rendering);

  query.style = Font::BOLD;
  params = GetFontRenderParams(query, NULL);
  EXPECT_EQ(FontRenderParams::HINTING_SLIGHT, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_NONE,
            params.subpixel_rendering);

  query.style = Font::ITALIC;
  params = GetFontRenderParams(query, NULL);
  EXPECT_EQ(FontRenderParams::HINTING_NONE, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_RGB,
            params.subpixel_rendering);

  query.style = Font::BOLD | Font::ITALIC;
  params = GetFontRenderParams(query, NULL);
  EXPECT_EQ(FontRenderParams::HINTING_NONE, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_NONE,
            params.subpixel_rendering);
}

TEST_F(FontRenderParamsTest, Scalable) {
  // Load a config that only enables antialiasing for scalable fonts.
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      kFontconfigMatchPatternHeader +
      CreateFontconfigEditStanza("antialias", "bool", "false") +
      kFontconfigMatchFooter +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("scalable", "eq", "bool", "true") +
      CreateFontconfigEditStanza("antialias", "bool", "true") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  // Check that we specifically ask how scalable fonts should be rendered.
  FontRenderParams params = GetFontRenderParams(
      FontRenderParamsQuery(false), NULL);
  EXPECT_TRUE(params.antialiasing);
}

TEST_F(FontRenderParamsTest, UseBitmaps) {
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("arial.ttf"));
  // Load a config that enables embedded bitmaps for fonts <= 10 pixels.
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      kFontconfigMatchPatternHeader +
      CreateFontconfigEditStanza("embeddedbitmap", "bool", "false") +
      kFontconfigMatchFooter +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("pixelsize", "less_eq", "double", "10") +
      CreateFontconfigEditStanza("embeddedbitmap", "bool", "true") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  FontRenderParamsQuery query(false);
  FontRenderParams params = GetFontRenderParams(query, NULL);
  EXPECT_FALSE(params.use_bitmaps);

  query.pixel_size = 5;
  params = GetFontRenderParams(query, NULL);
  EXPECT_TRUE(params.use_bitmaps);
}

TEST_F(FontRenderParamsTest, ForceFullHintingWhenAntialiasingIsDisabled) {
  // Load a config that disables antialiasing and hinting while requesting
  // subpixel rendering.
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      kFontconfigMatchPatternHeader +
      CreateFontconfigEditStanza("antialias", "bool", "false") +
      CreateFontconfigEditStanza("hinting", "bool", "false") +
      CreateFontconfigEditStanza("hintstyle", "const", "hintnone") +
      CreateFontconfigEditStanza("rgba", "const", "rgb") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  // Full hinting should be forced. See the comment in GetFontRenderParams() for
  // more information.
  FontRenderParams params = GetFontRenderParams(
      FontRenderParamsQuery(false), NULL);
  EXPECT_FALSE(params.antialiasing);
  EXPECT_EQ(FontRenderParams::HINTING_FULL, params.hinting);
  EXPECT_EQ(FontRenderParams::SUBPIXEL_RENDERING_NONE,
            params.subpixel_rendering);
  EXPECT_FALSE(params.subpixel_positioning);
}

#if defined(OS_CHROMEOS)
TEST_F(FontRenderParamsTest, ForceSubpixelPositioning) {
  {
    FontRenderParams params =
        GetFontRenderParams(FontRenderParamsQuery(false), NULL);
    EXPECT_TRUE(params.antialiasing);
    EXPECT_FALSE(params.subpixel_positioning);
    SetFontRenderParamsDeviceScaleFactor(1.0f);
  }
  ClearFontRenderParamsCacheForTest();
  SetFontRenderParamsDeviceScaleFactor(1.25f);
  // Subpixel positioning should be forced.
  {
    FontRenderParams params =
        GetFontRenderParams(FontRenderParamsQuery(false), NULL);
    EXPECT_TRUE(params.antialiasing);
    EXPECT_TRUE(params.subpixel_positioning);
    SetFontRenderParamsDeviceScaleFactor(1.0f);
  }
}
#endif

TEST_F(FontRenderParamsTest, OnlySetConfiguredValues) {
  // Configure the LinuxFontDelegate (which queries GtkSettings on desktop
  // Linux) to request subpixel rendering.
  FontRenderParams system_params;
  system_params.subpixel_rendering = FontRenderParams::SUBPIXEL_RENDERING_RGB;
  test_font_delegate_.set_params(system_params);

  // Load a Fontconfig config that enables antialiasing but doesn't say anything
  // about subpixel rendering.
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      kFontconfigMatchPatternHeader +
      CreateFontconfigEditStanza("antialias", "bool", "true") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  // The subpixel rendering setting from the delegate should make it through.
  FontRenderParams params = GetFontRenderParams(
      FontRenderParamsQuery(false), NULL);
  EXPECT_EQ(system_params.subpixel_rendering, params.subpixel_rendering);
}

TEST_F(FontRenderParamsTest, NoFontconfigMatch) {
  // Don't load a Fontconfig configuration.
  FontRenderParams system_params;
  system_params.antialiasing = true;
  system_params.hinting = FontRenderParams::HINTING_MEDIUM;
  system_params.subpixel_rendering = FontRenderParams::SUBPIXEL_RENDERING_RGB;
  test_font_delegate_.set_params(system_params);

  FontRenderParamsQuery query(false);
  query.families.push_back("Arial");
  query.families.push_back("Times New Roman");
  query.pixel_size = 10;
  std::string suggested_family;
  FontRenderParams params = GetFontRenderParams(query, &suggested_family);

  // The system params and the first requested family should be returned.
  EXPECT_EQ(system_params.antialiasing, params.antialiasing);
  EXPECT_EQ(system_params.hinting, params.hinting);
  EXPECT_EQ(system_params.subpixel_rendering, params.subpixel_rendering);
  EXPECT_EQ(query.families[0], suggested_family);
}

TEST_F(FontRenderParamsTest, MissingFamily) {
  // With Arial and Verdana installed, request (in order) Helvetica, Arial, and
  // Verdana and check that Arial is returned.
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("arial.ttf"));
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("verdana.ttf"));
  FontRenderParamsQuery query(false);
  query.families.push_back("Helvetica");
  query.families.push_back("Arial");
  query.families.push_back("Verdana");
  std::string suggested_family;
  GetFontRenderParams(query, &suggested_family);
  EXPECT_EQ("Arial", suggested_family);
}

TEST_F(FontRenderParamsTest, SubstituteFamily) {
  // Configure Fontconfig to use Verdana for both Helvetica and Arial.
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("arial.ttf"));
  ASSERT_TRUE(LoadSystemFontIntoFontconfig("verdana.ttf"));
  ASSERT_TRUE(LoadConfigDataIntoFontconfig(temp_dir_.path(),
      std::string(kFontconfigFileHeader) +
      CreateFontconfigAliasStanza("Helvetica", "Verdana") +
      kFontconfigMatchPatternHeader +
      CreateFontconfigTestStanza("family", "eq", "string", "Arial") +
      CreateFontconfigEditStanza("family", "string", "Verdana") +
      kFontconfigMatchFooter +
      kFontconfigFileFooter));

  FontRenderParamsQuery query(false);
  query.families.push_back("Helvetica");
  std::string suggested_family;
  GetFontRenderParams(query, &suggested_family);
  EXPECT_EQ("Verdana", suggested_family);

  query.families.clear();
  query.families.push_back("Arial");
  suggested_family.clear();
  GetFontRenderParams(query, &suggested_family);
  EXPECT_EQ("Verdana", suggested_family);
}

}  // namespace gfx
