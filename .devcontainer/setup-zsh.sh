#!/usr/bin/env bash
set -euo pipefail

USER_HOME="${HOME}"
ZSH_BIN="$(command -v zsh || true)"

if [[ -z "${ZSH_BIN}" ]]; then
  echo "zsh not installed; skipping."
  exit 0
fi

# Install oh-my-zsh if missing (unattended)
if [[ ! -d "${USER_HOME}/.oh-my-zsh" ]]; then
  echo "Installing oh-my-zsh..."
  export RUNZSH=no
  export CHSH=no
  sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended
fi

ZSH_CUSTOM="${ZSH_CUSTOM:-${USER_HOME}/.oh-my-zsh/custom}"
mkdir -p "${ZSH_CUSTOM}/plugins"

# Clone plugins if not present
if [[ ! -d "${ZSH_CUSTOM}/plugins/zsh-autosuggestions" ]]; then
  git clone https://github.com/zsh-users/zsh-autosuggestions "${ZSH_CUSTOM}/plugins/zsh-autosuggestions"
fi
if [[ ! -d "${ZSH_CUSTOM}/plugins/zsh-syntax-highlighting" ]]; then
  git clone https://github.com/zsh-users/zsh-syntax-highlighting "${ZSH_CUSTOM}/plugins/zsh-syntax-highlighting"
fi

ZSHRC="${USER_HOME}/.zshrc"
# Ensure plugins line includes desired plugins
if [[ -f "${ZSHRC}" ]]; then
  if grep -qE '^plugins=\(' "${ZSHRC}"; then
    sed -i 's/^plugins=.*/plugins=(git zsh-autosuggestions zsh-syntax-highlighting)/' "${ZSHRC}"
  else
    echo 'plugins=(git zsh-autosuggestions zsh-syntax-highlighting)' >> "${ZSHRC}"
  fi
else
  cat > "${ZSHRC}" <<'EOF'
export ZSH="$HOME/.oh-my-zsh"
ZSH_THEME="robbyrussell"
plugins=(git zsh-autosuggestions zsh-syntax-highlighting)
source $ZSH/oh-my-zsh.sh
EOF
fi

# Prefer zsh for interactive shells launched via bash
if ! grep -q 'exec zsh' "${USER_HOME}/.bashrc" 2>/dev/null; then
  echo 'export SHELL=/usr/bin/zsh' >> "${USER_HOME}/.bashrc"
  echo 'if [ -t 1 ]; then exec zsh; fi' >> "${USER_HOME}/.bashrc"
fi

echo "oh-my-zsh configured with git, autosuggestions, and syntax highlighting."
