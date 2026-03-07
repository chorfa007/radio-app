#!/bin/bash
# Creates RadioApp.app on your Desktop (macOS)
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="$SCRIPT_DIR/build/radio-app-gui"
APP="$HOME/Desktop/RadioApp.app"

if [ ! -f "$BINARY" ]; then
    echo "Binary not found — building first..."
    bash "$SCRIPT_DIR/start.sh"
fi

echo "Creating $APP ..."

# App bundle structure
mkdir -p "$APP/Contents/MacOS"
mkdir -p "$APP/Contents/Resources"

# Launcher script inside the bundle
cat > "$APP/Contents/MacOS/RadioApp" << LAUNCHER
#!/bin/bash
# Ensure Homebrew binaries (mpv) are on PATH when launched from Finder/Desktop
export PATH="/opt/homebrew/bin:/opt/homebrew/sbin:/usr/local/bin:\$PATH"
exec "$BINARY"
LAUNCHER
chmod +x "$APP/Contents/MacOS/RadioApp"

# Info.plist
cat > "$APP/Contents/Info.plist" << PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>       <string>RadioApp</string>
    <key>CFBundleIdentifier</key>       <string>com.radioapp.gui</string>
    <key>CFBundleName</key>             <string>Radio App</string>
    <key>CFBundleDisplayName</key>      <string>Radio App</string>
    <key>CFBundleVersion</key>          <string>1.0</string>
    <key>CFBundleShortVersionString</key><string>1.0</string>
    <key>CFBundlePackageType</key>      <string>APPL</string>
    <key>NSHighResolutionCapable</key>  <true/>
</dict>
</plist>
PLIST

# ── Generate icon ──────────────────────────────────────────────────────────
echo "Generating icon..."
cat > /tmp/make_icon.swift << 'SWIFT'
import AppKit, Foundation
let size: CGFloat = 512
let img = NSImage(size: NSSize(width: size, height: size))
img.lockFocus()
let ctx = NSGraphicsContext.current!.cgContext
let path = CGPath(roundedRect: CGRect(x:0,y:0,width:size,height:size),
                  cornerWidth:110,cornerHeight:110,transform:nil)
ctx.setFillColor(CGColor(red:0.22,green:0.06,blue:0.48,alpha:1))
ctx.addPath(path); ctx.fillPath()
let emoji = "📻" as NSString
let font = NSFont(name:"AppleColorEmoji",size:340) ?? NSFont.systemFont(ofSize:340)
emoji.draw(at:NSPoint(x:78,y:70),withAttributes:[.font:font])
img.unlockFocus()
let png = NSBitmapImageRep(data:img.tiffRepresentation!)!
    .representation(using:.png,properties:[:])!
try! png.write(to:URL(fileURLWithPath:"/tmp/radio-icon-512.png"))
SWIFT
swift /tmp/make_icon.swift

mkdir -p /tmp/RadioIcon.iconset
for size in 16 32 64 128 256 512; do
    sips -z $size $size /tmp/radio-icon-512.png --out /tmp/RadioIcon.iconset/icon_${size}x${size}.png > /dev/null
    double=$((size*2))
    [ $double -le 512 ] && sips -z $double $double /tmp/radio-icon-512.png \
        --out /tmp/RadioIcon.iconset/icon_${size}x${size}@2x.png > /dev/null
done
iconutil -c icns /tmp/RadioIcon.iconset -o "$APP/Contents/Resources/AppIcon.icns"
killall Dock 2>/dev/null || true

echo ""
echo "Done!  RadioApp.app is on your Desktop."
echo "Double-click it to launch, or drag it to your Dock."
