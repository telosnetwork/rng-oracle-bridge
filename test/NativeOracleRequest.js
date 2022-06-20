const { expect } = require("chai");
const { ethers } = require("hardhat");
const ONE_TLOS = ethers.utils.parseEther("1.0");

describe("NativeOracleRequest contract", function () {
    let bridge;
    beforeEach(async () => {
        const [owner] = await ethers.getSigners();
        let Bridge = await ethers.getContractFactory("NativeOracleRequest");
        bridge = await Bridge.deploy();
    })
    describe(":: Setup", function () {

    });
    describe(":: Core", function () {

    });
});
