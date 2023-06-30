# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
    config.ssh.forward_env = [ "ARMLMD_LICENSE_FILE" ]
    config.ssh.forward_x11 = true

    config.vm.provision "file", source: "~/.gitconfig", destination: "${HOME}/.gitconfig"

    config.vm.provider "docker" do |d|
        d.build_dir = "."

        d.build_args = [ "--target", "vagrant", "-f", "docker/Dockerfile" ]
        d.cmd = [ "sudo", "/usr/sbin/sshd", "-D" ]
        d.env = { "ARMLMD_LICENSE_FILE" => ENV["ARMLMD_LICENSE_FILE"] }

        d.has_ssh = true

        ENV["LC_ALL"] = "C.UTF-8"
    end
end
