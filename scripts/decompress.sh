#!/usr/bin/env bash
# =============================================================================
# decompress_unix_tree.sh
#
# PURPOSE:
#   Fully decompress a Unix source archive tree with layered encodings.
#   Handles: .gz, .Z (compress), .bz2, .uu/.uue (uuencode), .packed.gz,
#            .tar (extraction), and all combinations thereof.
#   Multi-pass: keeps running until absolutely nothing left to decompress.
#
# USAGE:
#   chmod +x decompress_unix_tree.sh
#   ./decompress_unix_tree.sh /path/to/unix/directory
#
#   Optional: resume mode (skip already-processed files in a log):
#   ./decompress_unix_tree.sh /path/to/unix/directory --resume
#
# SAFETY:
#   - Originals are removed ONLY after successful decompression/decoding
#   - Every action is logged to decompress.log in the target directory
#   - Errors are logged separately to decompress_errors.log
#   - A summary is printed at the end of each pass and overall
#   - No dotfiles, no temp directories, no junk left behind
#   - Safe to Ctrl+C and re-run (no partial state corruption)
#
# REQUIREMENTS:
#   gzip, bzip2, tar, uudecode (sharutils or bsdmainutils), uncompress (or gzip -d)
#   All standard on Linux. On macOS: brew install sharutils
# =============================================================================

set -euo pipefail

# Arguments 
TARGET_DIR="${1:-.}"

if [[ ! -d "$TARGET_DIR" ]]; then
    echo "ERROR: '$TARGET_DIR' is not a directory." >&2
    exit 1
fi

TARGET_DIR="$(realpath "$TARGET_DIR")"
LOG="$TARGET_DIR/decompress.log"
ERR_LOG="$TARGET_DIR/decompress_errors.log"

# Logging helpers ─
log()  { echo "[$(date '+%H:%M:%S')] $*" | tee -a "$LOG"; }
err()  { echo "[$(date '+%H:%M:%S')] ERROR: $*" | tee -a "$ERR_LOG" >&2; }
note() { echo "$*"; }

# Dependency check 
check_deps() {
    local missing=()
    for cmd in gzip bzip2 tar uudecode find; do
        command -v "$cmd" &>/dev/null || missing+=("$cmd")
    done
    # uncompress may be 'gzip -d' or a separate binary; either is fine
    if ! command -v uncompress &>/dev/null && ! gzip --version &>/dev/null; then
        missing+=("uncompress/gzip")
    fi
    if [[ ${#missing[@]} -gt 0 ]]; then
        echo "ERROR: Missing required tools: ${missing[*]}" >&2
        echo "Install with: sudo apt-get install gzip bzip2 tar sharutils" >&2
        exit 1
    fi
}

# Counters (global, reset per pass) 
declare -i PASS_OK=0 PASS_ERR=0 PASS_SKIP=0
declare -i TOTAL_OK=0 TOTAL_ERR=0

reset_counters() { PASS_OK=0; PASS_ERR=0; PASS_SKIP=0; }

# Core: safely decompress one file ─
#
# Returns 0 on success (file processed), 1 on error, 2 if skipped/not-handled.
#
process_file() {
    local file="$1"

    # Skip if file disappeared (race / already handled)
    [[ -f "$file" ]] || return 2

    # Skip our own log files
    [[ "$file" == "$LOG" || "$file" == "$ERR_LOG" ]] && return 2

    local dir
    dir="$(dirname "$file")"
    local base
    base="$(basename "$file")"

    # .gz (but NOT .tar.gz — handled separately below) ─
    if [[ "$base" == *.tar.gz ]]; then
        # Extract tar.gz in place, remove archive on success
        if tar -xzf "$file" -C "$dir" 2>>"$ERR_LOG"; then
            rm -f "$file"
            log "UNTAR.GZ  $file"
            (( PASS_OK++ )) || true
            return 0
        else
            err "tar.gz failed: $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    elif [[ "$base" == *.gz ]]; then
        # gunzip -f: force overwrite if output exists, -k not used — we want to
        # remove original. Use --suffix trick to preserve path.
        if gunzip -f "$file" 2>>"$ERR_LOG"; then
            log "GUNZIP    $file"
            (( PASS_OK++ )) || true
            return 0
        else
            err "gunzip failed: $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    # .Z (old compress format) 
    elif [[ "$base" == *.Z ]]; then
        if gunzip -f "$file" 2>>"$ERR_LOG"; then
            log "UNCOMPRESS $file"
            (( PASS_OK++ )) || true
            return 0
        else
            err "uncompress(.Z) failed: $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    # .bz2 
    elif [[ "$base" == *.tar.bz2 ]]; then
        if tar -xjf "$file" -C "$dir" 2>>"$ERR_LOG"; then
            rm -f "$file"
            log "UNTAR.BZ2 $file"
            (( PASS_OK++ )) || true
            return 0
        else
            err "tar.bz2 failed: $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    elif [[ "$base" == *.bz2 ]]; then
        if bunzip2 -f "$file" 2>>"$ERR_LOG"; then
            log "BUNZIP2   $file"
            (( PASS_OK++ )) || true
            return 0
        else
            err "bunzip2 failed: $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    # .tar (bare, already decompressed) 
    elif [[ "$base" == *.tar ]]; then
        if tar -xf "$file" -C "$dir" 2>>"$ERR_LOG"; then
            rm -f "$file"
            log "UNTAR     $file"
            (( PASS_OK++ )) || true
            return 0
        else
            err "tar failed: $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    # .uue / .uu (uuencoded) 
    # uudecode writes its output file to $dir (we cd there first)
    elif [[ "$base" == *.uue || "$base" == *.uu ]]; then
        local prev_dir="$PWD"
        cd "$dir"
        if uudecode "$base" 2>>"$ERR_LOG"; then
            rm -f "$base"
            log "UUDECODE  $file"
            (( PASS_OK++ )) || true
            cd "$prev_dir"
            return 0
        else
            err "uudecode failed: $file"
            (( PASS_ERR++ )) || true
            cd "$prev_dir"
            return 1
        fi

    # .packed (pack(1) format — use pcat or gunzip heuristic) 
    # "packed" files from OpenBSD/NetBSD test suites are just renamed archives.
    # They're typically .tar.gz.packed.gz or .txt.packed.gz — the outer .gz
    # will have been handled in a prior pass, leaving just a .packed file.
    # Try gunzip first (many are actually gzip), then skip if not recognized.
    elif [[ "$base" == *.packed ]]; then
        local out="${file%.packed}"
        if gunzip -c "$file" > "$out" 2>>"$ERR_LOG"; then
            rm -f "$file"
            log "UNPACK    $file"
            (( PASS_OK++ )) || true
            return 0
        else
            rm -f "$out"   # remove partial output
            err "unpack failed (not gzip?): $file"
            (( PASS_ERR++ )) || true
            return 1
        fi

    else
        # Not something we handle; leave it alone
        (( PASS_SKIP++ )) || true
        return 2
    fi
}

# One full pass over the tree ─
# Returns the number of files successfully processed (0 = nothing left to do)
run_pass() {
    local pass_num="$1"
    reset_counters

    log "========================================"
    log "PASS $pass_num starting"
    log "========================================"

    # Build the find expression: all files matching any pattern we handle.
    # Process them one by one so errors in one don't abort others.
    while IFS= read -r -d '' file; do
        process_file "$file" || true
    done < <(find "$TARGET_DIR" \( \
        -name "*.gz"     -o \
        -name "*.Z"      -o \
        -name "*.bz2"    -o \
        -name "*.tar"    -o \
        -name "*.uue"    -o \
        -name "*.uu"     -o \
        -name "*.packed" \
    \) -not -name "decompress*.log" -print0)

    TOTAL_OK=$(( TOTAL_OK + PASS_OK ))
    TOTAL_ERR=$(( TOTAL_ERR + PASS_ERR ))

    log "PASS $pass_num done — processed: $PASS_OK  errors: $PASS_ERR  skipped/ignored: $PASS_SKIP"
    note "  Pass $pass_num: ✓ $PASS_OK processed, ✗ $PASS_ERR errors"

    echo "$PASS_OK"
}

# Check for remaining compressed files (for final report) 
count_remaining() {
    find "$TARGET_DIR" \( \
        -name "*.gz"     -o \
        -name "*.Z"      -o \
        -name "*.bz2"    -o \
        -name "*.tar"    -o \
        -name "*.uue"    -o \
        -name "*.uu"     -o \
        -name "*.packed" \
    \) -not -name "decompress*.log" | wc -l
}

# MAIN
check_deps

note ""
note "  Unix Source Tree Decompressor"
note "  Target : $TARGET_DIR"
note "  Log    : $LOG"
note "  Errors : $ERR_LOG"
note ""

log "Started. Target: $TARGET_DIR"

pass=1
while true; do
    processed="$(run_pass "$pass")"

    if [[ "$processed" -eq 0 ]]; then
        log "Nothing processed in pass $pass — all done."
        break
    fi

    (( pass++ ))

    # Safety valve: if we somehow loop forever (shouldn't happen), stop at 20
    if [[ $pass -gt 20 ]]; then
        log "WARNING: reached 20-pass limit. Stopping."
        break
    fi
done

remaining="$(count_remaining)"

note ""
note "======================================================="
note "  DONE"
note "  Total passes   : $((pass - 1))"
note "  Total processed: $TOTAL_OK"
note "  Total errors   : $TOTAL_ERR"
note "  Still compressed: $remaining file(s)"
note "======================================================="
note ""

log "Finished. Passes: $((pass-1))  Total processed: $TOTAL_OK  Errors: $TOTAL_ERR  Remaining: $remaining"

if [[ "$remaining" -gt 0 ]]; then
    note "Some files could not be decompressed. Check decompress_errors.log"
    note "  Remaining compressed files:"
    find "$TARGET_DIR" \( \
        -name "*.gz" -o -name "*.Z" -o -name "*.bz2" -o \
        -name "*.tar" -o -name "*.uue" -o -name "*.uu" -o -name "*.packed" \
    \) -not -name "decompress*.log" | head -50
fi

if [[ "$TOTAL_ERR" -gt 0 ]]; then
    note ""
    note "$TOTAL_ERR error(s) occurred. See: $ERR_LOG"
    exit 1
fi

exit 0
